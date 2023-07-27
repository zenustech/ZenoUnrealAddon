// Zenustech © All Right Reserved


#include "ZenoLandscapeBundleActor.h"

#include "Landscape.h"
#include "LandscapeProxy.h"
#include "ScopedTransaction.h"
#include "ZenoLandscapeAsset.h"
#include "ZenoPointSetAsset.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "ZenoFoliageActor.h"

#define LOCTEXT_NAMESPACE "ZenoLandscapeBundleActor"

AZenoLandscapeBundleActor::AZenoLandscapeBundleActor(const FObjectInitializer& ObjectInitializer)
{
	RootComponent = Cast<USceneComponent>(ObjectInitializer.CreateDefaultSubobject(this, TEXT("RootComponent"), USceneComponent::StaticClass(), USceneComponent::StaticClass(), true, true));
}

void AZenoLandscapeBundleActor::BeginPlay()
{
	Super::BeginPlay();
}

void AZenoLandscapeBundleActor::AddOwned(const ALandscapeProxy* InLandscapeData)
{
	OwnedLandscapes.Add(InLandscapeData);
}

void AZenoLandscapeBundleActor::AddOwned(const AZenoFoliageActor* InPointSetData)
{
	OwnedFoliage.Add(InPointSetData);
}

#undef LOCTEXT_NAMESPACE
