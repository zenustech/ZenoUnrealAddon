// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Roles/LiveLinkTransformTypes.h"
#include "ZenoLiveLinkTypes.generated.h"

USTRUCT()
struct ZENOLIVELINK_API FLiveLinkHeightFieldStaticData : public FLiveLinkBaseStaticData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveLink")
	int64 Size;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveLink")
	TArray<float> Data;
	
};

USTRUCT(BlueprintType)
struct FLiveLinkHeightFieldBlueprintData : public  FLiveLinkBaseBlueprintData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveLink")
	FLiveLinkHeightFieldStaticData StaticData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveLink")
	FLiveLinkBaseFrameData FrameData;
};

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
