// Zenustech © All Right Reserved

#pragma once

#include "CoreMinimal.h"
#include "ZenoAssetBase.h"
#include "ZenoLandscapeAsset.generated.h"

UCLASS()
class ZENOENGINE_API UZenoLandscapeAsset : public UZenoAssetBase
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<uint16> HeightData;

	UPROPERTY(VisibleAnywhere)
	FIntPoint Extent = { 1, 1 };

	UPROPERTY(VisibleAnywhere)
	FZenoTransformInfo Transform;

	UPROPERTY(VisibleAnywhere)
	UTexture2D* BaseColorTexture = nullptr;

	friend class UZenoLandscapeAssetFactory;
};
