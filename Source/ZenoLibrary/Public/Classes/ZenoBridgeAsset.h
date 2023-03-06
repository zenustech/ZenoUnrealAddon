#pragma once

#include "CoreMinimal.h"
#include "ZenoTypes.h"
#include "UObject/Object.h"
#include "ZenoBridgeAsset.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class ZENOLIBRARY_API UZenoBridgeAsset : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION()
	inline EZenoAssetType GetAssetType() const { return AssetType; }

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Zeno")
	EZenoAssetType AssetType = EZenoAssetType::Invalid;
};
