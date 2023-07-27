// Zenustech © All Right Reserved

#pragma once

#include "CoreMinimal.h"
#include "ActorFactories/ActorFactory.h"
#include "ZenoLandscapeActorFactory.generated.h"

class AZenoFoliageActor;
class ALandscapeProxy;
class UZenoPointSetAsset;
class UZenoLandscapeAsset;
class AZenoLandscapeBundleActor;
/**
 * 
 */
UCLASS()
class ZENOEDITOR_API UZenoLandscapeActorFactory : public UActorFactory
{
	GENERATED_BODY()

	UZenoLandscapeActorFactory();

	virtual AActor* SpawnActor(UObject* InAsset, ULevel* InLevel, const FTransform& InTransform, const FActorSpawnParameters& InSpawnParams) override;

	virtual void PostSpawnActor(UObject* Asset, AActor* NewActor) override;
	
	virtual bool CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg) override;

	static ALandscapeProxy* AddLandscape(AZenoLandscapeBundleActor* NewActor, const UZenoLandscapeAsset* InLandscapeData);

	static AZenoFoliageActor* AddFoliage(AZenoLandscapeBundleActor* NewActor, const UZenoPointSetAsset* InPointSetData);
};
