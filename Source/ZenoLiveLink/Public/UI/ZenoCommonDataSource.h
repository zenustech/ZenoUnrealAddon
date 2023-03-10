#pragma once

#include "CoreMinimal.h"
#include "ILiveLinkClient.h"
#include "UObject/Object.h"
#include "LiveLinkTypes.h"
#include "ZenoLiveLinkSource.h"

/**
 * Utilities to access data from live link.
 */
class ZENOLIVELINK_API FZenoCommonDataSource
{
public:
	FZenoCommonDataSource();

	/**
	 * Check live link has connected to zeno
	 */
	static bool HasConnectToZeno();

	static TArray<FLiveLinkSubjectKey> GetAllSubjectKeys(bool bIncludeDisabled = false, bool bIncludeVirtual = false);

	template <
		typename T
	>
	static T* GetRoleChecked(const FLiveLinkSubjectKey& InSubjectKey);
	static TSubclassOf<ULiveLinkRole> GetRole(const FLiveLinkSubjectKey& InSubjectKey);

	static TOptional<FLiveLinkSubjectFrameData> GetFrameData(const FLiveLinkSubjectName SubjectName, const TSubclassOf<ULiveLinkRole> Role);
	static TOptional<FLiveLinkSubjectFrameData> GetFrameData(const FLiveLinkSubjectKey& SubjectKey);
	
private:
};

template <
	typename T
>
T* FZenoCommonDataSource::GetRoleChecked(const FLiveLinkSubjectKey& InSubjectKey)
{
	if (!HasConnectToZeno()) return nullptr;
	const TSubclassOf<ULiveLinkRole> SubjectRole = FZenoLiveLinkSource::CurrentProviderInstance->GetCurrentClient()->GetSubjectRole_AnyThread(InSubjectKey);
	return Cast<T>(SubjectRole);
}
