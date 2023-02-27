// Fill out your copyright notice in the Description page of Project Settings.

#define LOCTEXT_NAMESPACE "ZenoLiveLinkSource"

#include "ZenoLiveLinkSource.h"

#include "ILiveLinkClient.h"
#include "ZenoBridge.h"
#include "Kismet/KismetMathLibrary.h"
#include "Role/LiveLinkTranslationRole.h"
#include "Role/ZenoLiveLinkTypes.h"

FZenoLiveLinkSource::FZenoLiveLinkSource(const FZenoLiveLinkSetting InConnectionSettings)
	: Client(nullptr)
{
	SourceStatus = LOCTEXT("SourceStatus_NoData", "No data");
	SourceType = LOCTEXT("SourceType_Zeno", "Zeno");
	SourceMachineName = FText::Format(LOCTEXT("ZenoSourceMachineName", "{0}:{1}"), FText::FromString(InConnectionSettings.IPAddress), FText::AsNumber(InConnectionSettings.TCPPortNumber, &FNumberFormattingOptions::DefaultNoGrouping()));
}

FZenoLiveLinkSource::~FZenoLiveLinkSource()
{
}

void FZenoLiveLinkSource::ReceiveClient(ILiveLinkClient* InClient, const FGuid InSourceGuid)
{
	Client = InClient;
	SourceGuid = InSourceGuid;
}

bool FZenoLiveLinkSource::IsSourceStillValid() const
{
	return false;
}

bool FZenoLiveLinkSource::RequestSourceShutdown()
{
	FZenoBridgeModule& ZenoBridge = FModuleManager::Get().GetModuleChecked<FZenoBridgeModule>("ZenoBridge");
	ZenoBridge.StopClient();
	return false;
}

void FZenoLiveLinkSource::Update()
{
	FName SubjectName("Translation 1");
	
	FLiveLinkStaticDataStruct StaticData(FLiveLinkTranslationStaticData::StaticStruct());
	FLiveLinkTranslationStaticData& TranslationStaticData = *StaticData.Cast<FLiveLinkTranslationStaticData>();
	TranslationStaticData.bIsLocationSupported = false;
	TranslationStaticData.bIsRotationSupported = false;
	TranslationStaticData.bIsScaleSupported = true;
	TranslationStaticData.bIsInterpolation = true;
	Client->PushSubjectStaticData_AnyThread({ SourceGuid, SubjectName }, ULiveLinkTranslationRole::StaticClass(), MoveTemp(StaticData));

	FLiveLinkFrameDataStruct FrameData(FLiveLinkTranslationFrameData::StaticStruct());
	FLiveLinkTranslationFrameData* TranslationFrameData = FrameData.Cast<FLiveLinkTranslationFrameData>();
	TranslationFrameData->Offset = { 1.f, 1.f, 1.f };
	static double Cnt = 1.0;
	TranslationFrameData->Transform = FTransform::Identity;
	TranslationFrameData->Transform.ScaleTranslation(Cnt + 0.001);

	Client->PushSubjectFrameData_AnyThread({SourceGuid, SubjectName}, MoveTemp(FrameData));
}

#undef LOCTEXT_NAMESPACE
