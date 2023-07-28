// Zenustech © All Right Reserved


#include "ZenoLandscapeBundleActor.h"

#include "LandscapeProxy.h"
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

ALandscapeProxy* AZenoLandscapeBundleActor::GetFirstLandscape()
{
	for (const auto& Landscape : OwnedLandscapes)
	{
		if (!Landscape.IsNull())
		{
			return Landscape.LoadSynchronous();
		}
	}
	return nullptr;
}

#undef LOCTEXT_NAMESPACE
