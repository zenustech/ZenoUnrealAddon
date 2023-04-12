// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ZenoGraphAsset.generated.h"

UCLASS(BlueprintType)
class ZENOENGINE_API UZenoGraphAsset : public UObject
{
	GENERATED_BODY()

public:
	/** Raw JSON string exporting by zeno export (default hotkey: ctrl+shift+e.) */
	UPROPERTY(VisibleAnywhere, Category = "Zeno|Data", DisplayName = "Graph JSON")
	FString ZenoActionRecordExportedData;

	/** File path using to reimport from. */
	UPROPERTY(EditAnywhere, Category = "Zeno", DisplayName = "Reimport Path")
	FString ReimportPath;
};
