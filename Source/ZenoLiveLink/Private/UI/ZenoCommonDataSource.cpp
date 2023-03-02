#include "UI/ZenoCommonDataSource.h"

#include "ILiveLinkClient.h"
#include "ILiveLinkSource.h"
#include "LiveLinkController.h"
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
