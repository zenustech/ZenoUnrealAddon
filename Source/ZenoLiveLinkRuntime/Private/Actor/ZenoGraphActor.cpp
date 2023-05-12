// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/ZenoGraphActor.h"

#include "RawMesh.h"
#include "ZenoGraphAsset.h"
#include "Client/ZenoLiveLinkClientSubsystem.h"
#include "UObject/GCObjectScopeGuard.h"
#include "Utilities/ZenoEngineTypes.h"


AZenoGraphBaseActor::AZenoGraphBaseActor(const FObjectInitializer& ObjectInitializer)
{
	USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	AddOwnedComponent(SceneComponent);
	SetRootComponent(SceneComponent);
}

void AZenoGraphBaseActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AZenoGraphMeshActor::OnConstruction(const FTransform& Transform)
{
	if (IsValid(StaticMeshComponent))
	{
		StaticMeshComponent->AttachToComponent(RootComponent, { EAttachmentRule::KeepRelative, true });
	} 
}

#if WITH_EDITOR
void AZenoGraphMeshActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif // WITH_EDITOR

AZenoGraphMeshActor::AZenoGraphMeshActor(const FObjectInitializer& ObjectInitializer)
	: AZenoGraphBaseActor(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

#if WITH_EDITORONLY_DATA
	BuildSettings.bRecomputeNormals = true;
	BuildSettings.bRecomputeTangents = true;
	BuildSettings.bRemoveDegenerates = true;
	BuildSettings.bComputeWeightedNormals = false;
	BuildSettings.bUseMikkTSpace = false;
	BuildSettings.bUseFullPrecisionUVs = false;
	BuildSettings.bUseHighPrecisionTangentBasis = false;

	NaniteSettings.bEnabled = false;
#endif // WITH_EDITORONLY_DATA
}

#if WITH_EDITOR
void AZenoGraphMeshActor::SetMeshComponent(UStaticMeshComponent* InStaticMeshComponent)
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
	StaticMeshComponent->AttachToComponent(RootComponent, { EAttachmentRule::KeepRelative, true });
	RootComponent->RegisterComponent();
	UpdateComponentTransforms();
	Modify(false);
	PostEditChange();
}

void AZenoGraphMeshActor::OnGraphRunCompleted()
{
	if (IsValid(ZenoGraphAsset) && ZenoGraphAsset->OutputParameterDescriptors.Num() > 0)
	{
		// TODO [darc] : Handle multiple output parameters :
		const FZenoOutputParameterDescriptor& OutputParameter = ZenoGraphAsset->OutputParameterDescriptors[0];
		const FString SubjectName = OutputParameter.Name;
		const EZenoSubjectType SubjectType = OutputParameter.Type;
		if (SubjectType == EZenoSubjectType::Mesh)
		{
			using FRequiredType = zeno::remote::Mesh;
			UZenoLiveLinkClientSubsystem* ZenoClient = GEngine->GetEngineSubsystem<UZenoLiveLinkClientSubsystem>();
			ZenoClient->TryLoadSubjectRemotely<FRequiredType>(FName(SubjectName))->GetFuture().Then(
				[this] (const TResultFuture<FRequiredType>& Future)
				{
					if (!IsValid(this)) { return; }
					FGCObjectScopeGuard ScopeGuard(this);
					if (Future.Get().IsSet())
					{
						const zeno::remote::Mesh MeshData = Future.Get().GetValue();
						FRawMesh RawMesh = UZenoLiveLinkClientSubsystem::ConvertZenoMeshToRawMesh(MeshData);
						UStaticMesh* StaticMesh = NewObject<UStaticMesh>(this, UStaticMesh::StaticClass(), MakeUniqueObjectName(this, UStaticMesh::StaticClass(), FName("StaticMesh")), RF_Public|RF_Standalone);
						StaticMesh->PreEditChange(nullptr);
						StaticMesh->ImportVersion = LastVersion;
						StaticMesh->NaniteSettings = NaniteSettings;
						FStaticMeshSourceModel& SourceModel = StaticMesh->AddSourceModel();
						SourceModel.BuildSettings = BuildSettings;
						SourceModel.ReductionSettings = ReductionSettings;
						SourceModel.SaveRawMesh(RawMesh);
						StaticMesh->PostEditChange();

						UStaticMeshComponent* StaticMeshComponent = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), MakeUniqueObjectName(this, UStaticMeshComponent::StaticClass(), FName("StaticMeshComponent")), RF_Public|RF_Standalone);
						StaticMeshComponent->StaticMeshImportVersion = LastVersion;
						StaticMeshComponent->SetStaticMesh(StaticMesh);
						SetMeshComponent(StaticMeshComponent);
					}
				});
		}
	}
}
#endif // WITH_EDITOR

void AZenoGraphBaseActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

