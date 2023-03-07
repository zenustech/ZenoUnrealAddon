
#include "Landscape/Classes/ZenoLandscapeAnchor.h"

#include "ImageUtils.h"
#include "LandscapeEdit.h"
#include "LandscapeRender.h"
#include "LandscapeProxy.h"
#include "Classes/ZenoBridgeAsset.h"

#define LOCTEXT_NAMESPACE "AZenoLandscapeAnchor"

AZenoLandscapeAnchor::AZenoLandscapeAnchor(const FObjectInitializer& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
}

void AZenoLandscapeAnchor::OnConstruction(const FTransform& Transform)
{
}

#undef LOCTEXT_NAMESPACE
