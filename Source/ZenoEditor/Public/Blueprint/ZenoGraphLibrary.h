// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <memory>
#include <zeno/unreal/ZenoUnrealTypes.h>

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Utilities/ZenoEngineTypes.h"
#include "ZenoGraphLibrary.generated.h"

namespace zeno
{
	struct Graph;
}

class UZenoGraphAsset;

/**
 * 
 */
UCLASS()
class ZENOEDITOR_API UZenoGraphLibrary : public UObject
{
	GENERATED_BODY()

public:
	static std::shared_ptr<zeno::Graph> GetGraphFromJson(const char* Json);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static bool IsValidGraph(UZenoGraphAsset* Graph);

	static zeno::unreal::SubnetNodeParamList GetGraphParamList(UZenoGraphAsset* Graph);

	static EZenoParamType ConvertParamType(zeno::unreal::EParamType OriginParamType);
};
