// Fill out your copyright notice in the Description page of Project Settings.


#include "PCG/ZenoPCGVolume.h"

#include "RawMesh.h"
#include "Builders/CubeBuilder.h"
#include "Components/BrushComponent.h"
#include "Engine/Polys.h"

AZenoPCGVolume::AZenoPCGVolume(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SubjectName("ReservedName_Landscape")
{
	PCGComponent = ObjectInitializer.CreateDefaultSubobject<UZenoPCGVolumeComponent>(this, TEXT("PCG Component"));

	PolyFlags = 0;

	const UObject* That = this;
	while (That && That->HasAnyFlags(RF_DefaultSubObject))
	{
		That = That->GetOuter();
	}

	if (That && !That->HasAnyFlags(RF_ClassDefaultObject | RF_NeedLoad | RF_NeedPostLoad))
	{
		if (UBrushComponent* MyBrushComponent = GetBrushComponent())
		{
			MyBrushComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
			MyBrushComponent->SetMobility(EComponentMobility::Static);
			MyBrushComponent->SetGenerateOverlapEvents(false);
		}
	}
	SetActorScale3D({10.0, 10.0, 10.0});
}

#if WITH_EDITOR
void AZenoPCGVolume::PostActorCreated()
{
	Super::PostActorCreated();
	BuildBrush();
}

void AZenoPCGVolume::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	BuildBrush();
}
#endif // WITH_EDITOR

bool AZenoPCGVolume::GetReferencedContentObjects(TArray<UObject*>& Objects) const
{
	Super::GetReferencedContentObjects(Objects);

	if (PCGComponent)
	{
	}

	return true;
}

#if WITH_EDITOR
void AZenoPCGVolume::OnGeneratedNewMesh(FRawMesh& RawMesh)
{
	if (RawMesh.WedgeIndices.IsEmpty())
	{
		return;
	}
	UStaticMesh* StaticMesh = NewObject<UStaticMesh>(this, UStaticMesh::StaticClass(),
	                                                 MakeUniqueObjectName(
		                                                 this, UStaticMesh::StaticClass(), FName("StaticMesh")),
	                                                 RF_Public | RF_Standalone);
	StaticMesh->PreEditChange(nullptr);
	StaticMesh->ImportVersion = LastVersion;
	FStaticMeshSourceModel& SourceModel = StaticMesh->AddSourceModel();
	SourceModel.SaveRawMesh(RawMesh);
	StaticMesh->PostEditChange();

	UStaticMeshComponent* NewStaticMeshComponent = NewObject<UStaticMeshComponent>(
		this, UStaticMeshComponent::StaticClass(),
		MakeUniqueObjectName(this, UStaticMeshComponent::StaticClass(), FName("StaticMeshComponent")),
		RF_Public | RF_Standalone);
	NewStaticMeshComponent->StaticMeshImportVersion = LastVersion;
	NewStaticMeshComponent->SetStaticMesh(StaticMesh);
	SetStaticMeshComponent(NewStaticMeshComponent);
}

void AZenoPCGVolume::SetStaticMeshComponent(UStaticMeshComponent* InStaticMeshComponent)
{
	PreEditChange(nullptr);
	if (!IsValid(InStaticMeshComponent) || StaticMeshComponent == InStaticMeshComponent)
		return;
	if (IsValid(StaticMeshComponent))
	{
		StaticMeshComponent->DestroyComponent(false);
		RemoveOwnedComponent(StaticMeshComponent);
		StaticMeshComponent->ReleaseResources();
	}
	StaticMeshComponent = InStaticMeshComponent;
	AddOwnedComponent(InStaticMeshComponent);
	StaticMeshComponent->AttachToComponent(GetRootComponent(), { EAttachmentRule::SnapToTarget, true });
	StaticMeshComponent->RegisterComponent();
	UpdateComponentTransforms();

	FBox MeshBound = StaticMeshComponent->CalcBounds(StaticMeshComponent->GetComponentToWorld()).GetBox();
	const FBox VolumeBox = GetBrushComponent()->CalcBounds(GetBrushComponent()->GetComponentToWorld()).GetBox();
	// Scale the mesh to fit the volume
	const FVector VolumeSize = VolumeBox.GetSize();
	const FVector MeshSize = MeshBound.GetSize();
	FVector Scale = VolumeSize / MeshSize;
	Scale.Z = 1.0f;
	StaticMeshComponent->SetRelativeScale3D(Scale);
	// Update the mesh bounds
	MeshBound = StaticMeshComponent->CalcBounds(StaticMeshComponent->GetComponentToWorld()).GetBox();
	// Move mesh origin to the center of the volume
	const FVector MeshCenter = MeshBound.GetCenter();
	const FVector VolumeCenter = VolumeBox.GetCenter();
	const FVector Offset = VolumeCenter - MeshCenter;
	StaticMeshComponent->AddWorldOffset(Offset);
	
	Modify(false);
	PostEditChange();
}

void AZenoPCGVolume::BuildBrush()
{
	Brush = NewObject<UModel>(this, NAME_None, RF_Transactional);
	Brush->Initialize(nullptr, true);
	Brush->Polys = NewObject<UPolys>(this, NAME_None, RF_Transactional);
	GetBrushComponent()->Brush = Brush;
	BrushBuilder = NewObject<UCubeBuilder>(this, NAME_None, RF_Transactional);

	UCubeBuilder* CubeBuilder = static_cast<UCubeBuilder*>(BrushBuilder);

	CubeBuilder->X = GetActorScale().X;
	CubeBuilder->Y = GetActorScale().Y;
	CubeBuilder->Z = GetActorScale().Z;

	BrushBuilder->Build(GetWorld(), this);
}
#endif // WITH_EDITOR
