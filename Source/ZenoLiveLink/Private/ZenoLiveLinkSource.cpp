// Fill out your copyright notice in the Description page of Project Settings.

#define LOCTEXT_NAMESPACE "ZenoLiveLinkSource"

#include "ZenoLiveLinkSource.h"

#include "ILiveLinkClient.h"
#include "ZenoBridge.h"
#include "Role/LiveLinkTranslationRole.h"
#include "Role/ZenoLiveLinkTypes.h"
#include "model/networktypes.h"
#include "model/subject.h"
#include "3rd/msgpack.h"
#include "Role/LiveLinkTextureRole.h"

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
	return true;
}

void FZenoLiveLinkSource::Update()
{
}

void FZenoLiveLinkSource::OnReceivedNewFile(const ZBFileType FileType, const TArray<uint8>& RawData)
{
	if (FileType == ZBFileType::HeightField)
	{
		std::error_code Error;
		const UnrealHeightFieldSubject Subject = msgpack::unpack<UnrealHeightFieldSubject>(RawData.GetData(), RawData.Num(), Error);
		if (Error)
		{
			UE_LOG(LogTemp, Warning, TEXT("Unable to unpack file: %hs"), Error.message().c_str());
			return;
		}
		const FName SubjectName(Subject.m_name.c_str());
		EncounteredSubjects.Add(SubjectName);
		
		FLiveLinkStaticDataStruct StaticData(FLiveLinkHeightFieldStaticData::StaticStruct());
		FLiveLinkHeightFieldStaticData& HeightFieldStaticData = *StaticData.Cast<FLiveLinkHeightFieldStaticData>();
		HeightFieldStaticData.Size = Subject.m_resolution;
		HeightFieldStaticData.Data.SetNumUninitialized(Subject.m_height.size());
		std::memmove(HeightFieldStaticData.Data.GetData(), Subject.m_height.data(), Subject.m_height.size());
		Client->PushSubjectStaticData_AnyThread({ SourceGuid, SubjectName }, ULiveLinkHeightFieldRole::StaticClass(), MoveTemp(StaticData));
	}
}

#undef LOCTEXT_NAMESPACE
