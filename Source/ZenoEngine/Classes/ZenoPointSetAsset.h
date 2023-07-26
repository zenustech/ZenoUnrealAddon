// Zenustech © All Right Reserved

#pragma once

#include "CoreMinimal.h"
#include "ZenoAssetBase.h"
#include "ZenoPointSetAsset.generated.h"

/** zeno::unreal::PointSet::Type in UE side */
UENUM()
enum class EZenoPointType
{
	Misc,
	Tree,
	Grass,
};

UCLASS()
class ZENOENGINE_API UZenoPointSetAsset : public UZenoAssetBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere)
	TArray<FVector> Points;

	UPROPERTY(VisibleAnywhere)
	EZenoPointType PointType = EZenoPointType::Misc;

	friend class UZenoLandscapeAssetFactory;
};
