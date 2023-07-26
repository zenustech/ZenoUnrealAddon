#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ZenoAssetTypes.generated.h"

UCLASS()
class ZENOENGINE_API UZenoAssetTypesLibrary : public UObject
{
	GENERATED_BODY()
};

UENUM()
enum class EBundleDataType
{
	Invalid = -1,
	Landscape = 0 UMETA(Default),
	Marker,
	Mesh,
	Instance,
	Num UMETA(Hidden),
};

