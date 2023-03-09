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
	if (CurrentProviderInstance.Get() == this)
	{
		CurrentProviderInstance.Reset();
	}
	
	FZenoBridgeModule& ZenoBridge = FModuleManager::Get().GetModuleChecked<FZenoBridgeModule>("ZenoBridge");
	ZenoBridge.StopClient();
	return true;
}

void FZenoLiveLinkSource::Update()
{
	if (const int32 Num = EncounteredSubjects.Num(); Num == 0)
	{
		SourceStatus = LOCTEXT("SourceStatus_NoData", "No data");
	} else
	{
		SourceStatus = FText::Format(LOCTEXT("SourceStatus_Subject", "{0} Subjects"), Num);
	}
	for (const auto Subject : EncounteredSubjects)
	{
		FLiveLinkFrameDataStruct FrameData(FLiveLinkBaseFrameData::StaticStruct());
		Client->PushSubjectFrameData_AnyThread({ SourceGuid, Subject }, MoveTemp(FrameData));
	}
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
		// const FName SubjectName(Subject.m_name.substr(0, Subject.m_name.find(":")).c_str() );
		const FName SubjectName(Subject.m_name.c_str());
		EncounteredSubjects.Add(SubjectName);
		
		FLiveLinkStaticDataStruct StaticData(FLiveLinkHeightFieldStaticData::StaticStruct());
		FLiveLinkHeightFieldStaticData& HeightFieldStaticData = *StaticData.Cast<FLiveLinkHeightFieldStaticData>();
		HeightFieldStaticData.Size = Subject.m_resolution;
		HeightFieldStaticData.Data.SetNumUninitialized(Subject.m_height.size());
		// for (size_t Idx = 0; Idx < Subject.m_height.size(); ++Idx)
		// {
		// 	HeightFieldStaticData.Data[Idx] = Subject.m_height[Idx];
		// }
		FGenericPlatformMemory::Memmove(HeightFieldStaticData.Data.GetData(), Subject.m_height.data(), Subject.m_height.size() * sizeof(float));
		Client->PushSubjectStaticData_AnyThread({ SourceGuid, SubjectName }, ULiveLinkHeightFieldRole::StaticClass(), MoveTemp(StaticData));
	}
}

ILiveLinkClient* FZenoLiveLinkSource::GetCurrentClient() const
{
	return Client;
}

FGuid FZenoLiveLinkSource::GetGuid() const
{
	return SourceGuid;
}

bool FZenoLiveLinkSource::HasSubject(const FName SubjectName) const
{
	return EncounteredSubjects.Contains(SubjectName);
}

#undef LOCTEXT_NAMESPACE
