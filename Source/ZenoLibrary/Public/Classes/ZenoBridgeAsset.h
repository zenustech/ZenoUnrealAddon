#pragma once

#include "CoreMinimal.h"
#include "ZenoTypes.h"
#include "UObject/Object.h"
#include "ZenoBridgeAsset.generated.h"

class UZenoBridgeAssetData_Base;

UCLASS(BlueprintType)
class ZENOLIBRARY_API UZenoBridgeAsset : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION()
	inline EZenoAssetType GetAssetType() const { return AssetType; }

	UFUNCTION()
	inline void SetAssetType(const EZenoAssetType Type) { AssetType = Type; }

	UFUNCTION()
	inline UZenoBridgeAssetData_Base* GetAssetData() const { return AssetData; }

	UFUNCTION()
	inline void SetAssetData(UZenoBridgeAssetData_Base* Data) { AssetData = Data; }

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Zeno")
	EZenoAssetType AssetType = EZenoAssetType::Invalid;

	UPROPERTY(BlueprintReadOnly, Category = "Zeno")
	UZenoBridgeAssetData_Base* AssetData;
};

UCLASS(BlueprintType)
class ZENOLIBRARY_API UZenoBridgeAssetData_Base : public UObject
{
	GENERATED_BODY()

	friend UZenoBridgeAsset;
};

UCLASS(BlueprintType)
class ZENOLIBRARY_API UZenoBridgeAssetData_Heightfield : public UZenoBridgeAssetData_Base
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	int32 Size = 0;

	UPROPERTY()
	TArray<uint16> Heights;
};
