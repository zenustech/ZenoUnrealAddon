// Zenustech © All Right Reserved


#include "ZenoLandscapeBundleActor.h"

#include "LandscapeProxy.h"
#include "ZenoFoliageActor.h"

#define LOCTEXT_NAMESPACE "ZenoLandscapeBundleActor"

AZenoLandscapeBundleActor::AZenoLandscapeBundleActor(const FObjectInitializer& ObjectInitializer)
{
	RootComponent = Cast<USceneComponent>(ObjectInitializer.CreateDefaultSubobject(this, TEXT("RootComponent"), USceneComponent::StaticClass(), USceneComponent::StaticClass(), true, true));
	RootComponent->SetFlags(RF_Public);
	RootComponent->SetMobility(EComponentMobility::Static);
}

void AZenoLandscapeBundleActor::BeginPlay()
{
	Super::BeginPlay();
}

void AZenoLandscapeBundleActor::PostLoad()
{
	Super::PostLoad();
	if (!RootComponent->HasAnyFlags(RF_Public))
	{
		RootComponent->SetFlags(RF_Public);
	}
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
