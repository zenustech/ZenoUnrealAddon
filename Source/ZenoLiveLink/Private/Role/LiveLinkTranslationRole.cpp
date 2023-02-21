// Fill out your copyright notice in the Description page of Project Settings.

#define LOCTEXT_NAMESPACE "LiveLinkRole"

#include "Role/LiveLinkTranslationRole.h"

#include "Role/ZenoLiveLinkTypes.h"

UScriptStruct* ULiveLinkTranslationRole::GetStaticDataStruct() const
{
	return FLiveLinkTranslationStaticData::StaticStruct();
}

UScriptStruct* ULiveLinkTranslationRole::GetFrameDataStruct() const
{
	return FLiveLinkTranslationFrameData::StaticStruct();
}

UScriptStruct* ULiveLinkTranslationRole::GetBlueprintDataStruct() const
{
	return FLiveLinkTranslationBlueprintData::StaticStruct();
}

bool ULiveLinkTranslationRole::InitializeBlueprintData(const FLiveLinkSubjectFrameData& InSourceData,
	FLiveLinkBlueprintDataStruct& OutBlueprintData) const
{
	FLiveLinkTranslationBlueprintData* BlueprintData = OutBlueprintData.Cast<FLiveLinkTranslationBlueprintData>();
	const FLiveLinkTranslationStaticData* StaticData = InSourceData.StaticData.Cast<FLiveLinkTranslationStaticData>();
	const FLiveLinkTranslationFrameData* FrameData = InSourceData.FrameData.Cast<FLiveLinkTranslationFrameData>();

	if (BlueprintData && StaticData && FrameData)
	{
		GetStaticDataStruct()->CopyScriptStruct(&BlueprintData->StaticData, StaticData);
		GetFrameDataStruct()->CopyScriptStruct(&BlueprintData->FrameData, FrameData);
		return true;
	}
	
	return false;
}

FText ULiveLinkTranslationRole::GetDisplayName() const
{
	return LOCTEXT("Translation", "Translation");
}

#undef LOCTEXT_NAMESPACE
