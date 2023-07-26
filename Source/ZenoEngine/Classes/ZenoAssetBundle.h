#pragma once

#include "CoreMinimal.h"
#include "ZenoAssetBase.h"
#include "UObject/Object.h"
#include "ZenoAssetBundle.generated.h"

UCLASS()
class ZENOENGINE_API UZenoAssetBundle : public UZenoAssetBase
{
	GENERATED_BODY()

public:
	explicit UZenoAssetBundle(const FObjectInitializer& ObjectInitializer);
	
};
