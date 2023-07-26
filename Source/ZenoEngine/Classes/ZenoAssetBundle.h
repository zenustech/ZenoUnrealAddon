#pragma once

#include "CoreMinimal.h"
#include "ZenoAssetBase.h"
#include "UObject/Object.h"
#include "ZenoAssetBundle.generated.h"

class UZenoLandscapeAsset;
class UZenoPointSetAsset;

UCLASS()
class ZENOENGINE_API UZenoAssetBundle : public UZenoAssetBase
{
	GENERATED_BODY()

public:
	explicit UZenoAssetBundle(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(VisibleAnywhere)
	TArray<UZenoPointSetAsset*> PointSet;

	UPROPERTY(VisibleAnywhere)
	TArray<UZenoLandscapeAsset*> Landscapes;

	UPROPERTY(VisibleAnywhere)
	TArray<UTexture2D*> UsedTextures;

	UPROPERTY(VisibleAnywhere)
	TMap<FString, UZenoAssetBase*> NameToAssetMap;
	
	friend class UZenoLandscapeAssetFactory;
};
