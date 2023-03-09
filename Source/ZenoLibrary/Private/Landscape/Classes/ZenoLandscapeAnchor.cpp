
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
	if (IsValid(Heightfield))
	{
		if (UWorld* World = GetWorld())
		{
			LandscapeActor = World->SpawnActor<ALandscape>(FVector::ZeroVector, FRotator::ZeroRotator);
			if (IsValid(LandscapeActor))
			{
				LandscapeActor->AttachToActor(this, FAttachmentTransformRules { EAttachmentRule::KeepWorld, false });
				LandscapeActor->SetActorScale3D(LandscapeScale);
			}
		}
		// const int32 SizeX = 1 * 127 + 1;
		// const int32 SizeY = 1 * 127 + 1;
		const int32 Size = sqrt(Heightfield->Size);

		TMap<FGuid, TArray<uint16>> HeightDataPerLayers;
		TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayers;
		TArray<FLandscapeImportLayerInfo> LandscapeImportLayerInfos;

		HeightDataPerLayers.Add({ FGuid(), Heightfield->Heights });
		MaterialLayerDataPerLayers.Add({FGuid(), LandscapeImportLayerInfos});
		
		LandscapeActor->Import(FGuid::NewGuid(), 0, 0, Size - 1, Size - 1, 1, 7, HeightDataPerLayers, TEXT(""), MaterialLayerDataPerLayers, ELandscapeImportAlphamapType::Additive);
	}
}

#undef LOCTEXT_NAMESPACE
