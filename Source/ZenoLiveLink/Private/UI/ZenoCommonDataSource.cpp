﻿#include "UI/ZenoCommonDataSource.h"

#include "ILiveLinkClient.h"
#include "ILiveLinkSource.h"
#include "ZenoLiveLinkSource.h"

UZenoCommonDataSource::UZenoCommonDataSource()
{
}

bool UZenoCommonDataSource::HasConnectToZeno()
{
	return FZenoLiveLinkSource::CurrentProviderInstance.IsValid();
}

TArray<FLiveLinkSubjectKey> UZenoCommonDataSource::GetAllSubjectKeys(const bool bIncludeDisabled, const bool bIncludeVirtual)
{
	if (!HasConnectToZeno()) return {};
	return FZenoLiveLinkSource::CurrentProviderInstance->GetCurrentClient()->GetSubjects(bIncludeDisabled, bIncludeVirtual);
}

TSubclassOf<ULiveLinkRole> UZenoCommonDataSource::GetRole(const FLiveLinkSubjectKey& InSubjectKey)
{
	if (!HasConnectToZeno()) return nullptr;
	return FZenoLiveLinkSource::CurrentProviderInstance->GetCurrentClient()->GetSubjectRole_AnyThread(InSubjectKey);
}

TOptional<FLiveLinkSubjectFrameData> UZenoCommonDataSource::GetFrameData(const FLiveLinkSubjectName SubjectName,
                                                                         const TSubclassOf<ULiveLinkRole> Role)
{
	if (!HasConnectToZeno()) return {};
	FLiveLinkSubjectFrameData Data;
	FZenoLiveLinkSource::CurrentProviderInstance->GetCurrentClient()->EvaluateFrame_AnyThread(SubjectName, Role, Data);
	return { std::move(Data) };
}

TOptional<FLiveLinkSubjectFrameData> UZenoCommonDataSource::GetFrameData(const FLiveLinkSubjectKey& SubjectKey)
{
	if (!HasConnectToZeno()) return {};
	const TSubclassOf<ULiveLinkRole> Role = GetRole(SubjectKey);
	return GetFrameData(SubjectKey.SubjectName, Role);
}

