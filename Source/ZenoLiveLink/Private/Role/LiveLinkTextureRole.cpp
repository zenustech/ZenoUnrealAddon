// Fill out your copyright notice in the Description page of Project Settings.


#include "Role/LiveLinkTextureRole.h"

#include "Role/ZenoLiveLinkTypes.h"

#define LOCTEXT_NAMESPACE "LiveLinkRole"

UScriptStruct* ULiveLinkHeightFieldRole::GetStaticDataStruct() const
{
	return FLiveLinkHeightFieldStaticData::StaticStruct();
}

UScriptStruct* ULiveLinkHeightFieldRole::GetFrameDataStruct() const
{
	return Super::GetFrameDataStruct();
}

UScriptStruct* ULiveLinkHeightFieldRole::GetBlueprintDataStruct() const
{
	return Super::GetBlueprintDataStruct();
}

bool ULiveLinkHeightFieldRole::InitializeBlueprintData(const FLiveLinkSubjectFrameData& InSourceData,
	FLiveLinkBlueprintDataStruct& OutBlueprintData) const
{
	FLiveLinkHeightFieldBlueprintData* BlueprintData = OutBlueprintData.Cast<FLiveLinkHeightFieldBlueprintData>();
	const FLiveLinkHeightFieldStaticData* StaticData = InSourceData.StaticData.Cast<FLiveLinkHeightFieldStaticData>();
	const FLiveLinkBaseFrameData* FrameData = InSourceData.FrameData.Cast<FLiveLinkBaseFrameData>();

	if (BlueprintData && StaticData && FrameData)
	{
		GetStaticDataStruct()->CopyScriptStruct(&BlueprintData->StaticData, StaticData);
		GetFrameDataStruct()->CopyScriptStruct(&BlueprintData->FrameData, FrameData);
		return true;
	}
	return false;
}

FText ULiveLinkHeightFieldRole::GetDisplayName() const
{
	return LOCTEXT("HeightField", "HeightField");
}

#undef LOCTEXT_NAMESPACE
