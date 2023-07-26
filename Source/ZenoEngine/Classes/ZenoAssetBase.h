#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ZenoAssetBase.generated.h"

UCLASS(Abstract)
class ZENOENGINE_API UZenoAssetBase : public UObject
{
	GENERATED_BODY()
};

USTRUCT()
struct FZenoTransformInfo
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	FVector Position;
	UPROPERTY(VisibleAnywhere)
	FVector Rotation;
	UPROPERTY(VisibleAnywhere)
	FVector Scale;
};
