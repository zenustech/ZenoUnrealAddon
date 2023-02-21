// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Roles/LiveLinkTransformTypes.h"
#include "ZenoLiveLinkTypes.generated.h"

USTRUCT(BlueprintType)
struct ZENOLIVELINK_API FLiveLinkTranslationStaticData : public FLiveLinkTransformStaticData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveLink")
	bool bIsInterpolation = false;
};

USTRUCT(BlueprintType)
struct FLiveLinkTranslationFrameData : public FLiveLinkTransformFrameData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveLink", Interp)
	FVector Offset;
};

USTRUCT(BlueprintType)
struct FLiveLinkTranslationBlueprintData : public  FLiveLinkBaseBlueprintData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveLink")
	FLiveLinkTranslationStaticData StaticData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveLink")
	FLiveLinkTranslationFrameData FrameData;
};
