// Fill out your copyright notice in the Description page of Project Settings.


#include "ZenoGraphActor.h"


AZenoGraphBaseActor::AZenoGraphBaseActor(const FObjectInitializer& ObjectInitializer)
{
}

void AZenoGraphBaseActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AZenoGraphMeshActor::OnConstruction(const FTransform& Transform)
{
	StaticMeshComponent = NewObject<UStaticMeshComponent>(this);
	SetRootComponent(StaticMeshComponent);
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
}

void AZenoGraphBaseActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

