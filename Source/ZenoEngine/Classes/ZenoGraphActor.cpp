// Fill out your copyright notice in the Description page of Project Settings.


#include "ZenoGraphActor.h"


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
	if (StaticMeshComponent == InStaticMeshComponent)
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
#endif // WITH_EDITOR

void AZenoGraphBaseActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

