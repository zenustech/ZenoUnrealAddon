// Fill out your copyright notice in the Description page of Project Settings.


#include "PCG/ZenoPCGVolume.h"

#include "EditorSupportDelegates.h"
#include "RawMesh.h"
#include "Builders/CubeBuilder.h"
#include "Client/ZenoLiveLinkClientSubsystem.h"
#include "Components/BrushComponent.h"
#include "Engine/Polys.h"
#include "PCG/ZenoPCGVolumeComponent.h"
#include "UObject/GCObjectScopeGuard.h"
#include "ZenoGraphAsset.h"
#include "Input/ZenoInputParameter.h"

AZenoPCGVolume::AZenoPCGVolume(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SubjectName("ReservedName_Landscape")
{
	PCGComponent = ObjectInitializer.CreateDefaultSubobject<UZenoPCGVolumeComponent>(this, TEXT("PCG Component"));
	PCGComponent->ParameterChangedEvent.AddUObject(this, &AZenoPCGVolume::ExecutePCGGraph);

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
void AZenoPCGVolume::OnGeneratedNewMesh(FRawMesh& RawMesh, const FVector4f& InBoundDiff)
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
	SetStaticMeshComponent(NewStaticMeshComponent, InBoundDiff);
}

void AZenoPCGVolume::SetStaticMeshComponent(UStaticMeshComponent* InStaticMeshComponent, const FVector4f& InBoundDiff)
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
	// Adjust by the bound difference
	Scale.X = Scale.X * (1 - FMath::Abs(InBoundDiff.X) - FMath::Abs(InBoundDiff.Y));
	Scale.Y = Scale.Y * (1 - FMath::Abs(InBoundDiff.Z) - FMath::Abs(InBoundDiff.W));
	Scale.Z = 1.0f;
	StaticMeshComponent->SetRelativeScale3D(Scale);
	// Update the mesh bounds
	MeshBound = StaticMeshComponent->CalcBounds(StaticMeshComponent->GetComponentToWorld()).GetBox();
	// Move mesh origin to the center of the volume
	const FVector MeshCenter = MeshBound.GetCenter();
	const FVector VolumeCenter = VolumeBox.GetCenter();
	FVector Offset = VolumeCenter - MeshCenter;
	Offset.X = Offset.X - (Offset.X * InBoundDiff.Y);
	Offset.Y = Offset.Y - (Offset.Y * InBoundDiff.W);
	StaticMeshComponent->AddWorldOffset(Offset);
	
	Modify(false);
	PostEditChange();
}

void AZenoPCGVolume::ExecutePCGGraph()
{
	AZenoPCGVolume* Volume = this;
	UZenoLiveLinkClientSubsystem* Client = GEngine->GetEngineSubsystem<UZenoLiveLinkClientSubsystem>();
	const auto HeightFieldPtr = Volume->PCGComponent->GetLandscapeHeightData();
	if (!HeightFieldPtr)
	{
		return;
	}
	if (const UZenoLiveLinkSession* Session = Client->GetSessionFallback(); IsValid(Session))
	{
		// Push height field to zeno remote
		zeno::remote::HeightField HeightField = *HeightFieldPtr;
		zeno::remote::SubjectContainer Subject{
			{TCHAR_TO_ANSI(*Volume->SubjectName)},
			static_cast<uint16>(zeno::remote::ESubjectType::HeightField),
			msgpack::pack(HeightField),
		};
		zeno::remote::SubjectContainerList List{
			{Subject,},
		};
		Session->GetClient()->SetSubjectToRemote(List)->GetFuture()
		       .Then([Session, Volume](const TResultFuture<bool>& Future)
		       {
			       if (Future.Get().Get(false) && IsValid(Session) && IsValid(Volume) && IsValid(Volume->PCGComponent))
			       {
				       FGCObjectScopeGuard SessionGuard{Session};
				       FGCObjectScopeGuard VolumeGuard{Volume};
				       zeno::remote::GraphRunInfo RunInfo;
				       const UZenoGraphAsset* GraphAsset = Volume->PCGComponent->ZenoGraph.LoadSynchronous();
				       RunInfo.GraphDefinition = TCHAR_TO_ANSI(*GraphAsset->ZenoActionRecordExportedData);
				       for (const UZenoInputParameter* Input : Volume->PCGComponent->InputParameters)
				       {
					       RunInfo.Values.Values.push_back(Input->GatherParamValue());
				       }
				       Session->GetClient()->RunGraph(RunInfo)->GetFuture()
				              .Then([Session, Volume](const TResultFuture<bool>& RunFuture)
				              {
					              if (RunFuture.Get().Get(false) && IsValid(Session) && IsValid(Volume))
					              {
						              FGCObjectScopeGuard SessionGuard1{Session};
						              FGCObjectScopeGuard VolumeGuard1{Volume};
						              // Load result mesh from zeno remote
						              const UZenoGraphAsset* GraphInfo = Volume->PCGComponent->ZenoGraph.
							              LoadSynchronous();
						              if (!GraphInfo->OutputParameterDescriptors.IsEmpty())
						              {
							              GraphInfo->ForEachOutputDescriptor<EZenoSubjectType::Mesh>(FZenoOutputDescriptorDelegate::CreateLambda([Volume] (const FZenoOutputParameterDescriptor& Descriptor)
							              {
								              const FString ResultName = Descriptor.Name;
								              UZenoLiveLinkClientSubsystem* LiveLinkSubsystem = GEngine->
									              GetEngineSubsystem<UZenoLiveLinkClientSubsystem>();
								              LiveLinkSubsystem->TryLoadSubjectRemotely<zeno::remote::Mesh>(
									                               FName(ResultName))->GetFuture()
								                               .Then([Volume](
									                               const TResultFuture<zeno::remote::Mesh>& MeshFuture)
									                               {
										                               if (MeshFuture.Get().IsSet() && IsValid(Volume))
										                               {
											                               FGCObjectScopeGuard VolumeGuard2{Volume};
											                               const zeno::remote::Mesh MeshData =
												                               MeshFuture.Get().GetValue();
											                               std::array<float, 4> BoundDiff = MeshData.
												                               GetBoundDiff();
											                               FRawMesh RawMesh =
												                               UZenoLiveLinkClientSubsystem::ConvertZenoMeshToRawMesh(
													                               MeshData);
											                               Volume->OnGeneratedNewMesh(
												                               RawMesh, FVector4f{
													                               BoundDiff[0], BoundDiff[1],
													                               BoundDiff[3], BoundDiff[2]
												                               });
										                               }
									                               });
							              }));
						              }
					              }
				              });
			       }
		       });
	}
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
