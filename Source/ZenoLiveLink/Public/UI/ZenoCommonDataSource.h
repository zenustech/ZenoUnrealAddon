#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "LiveLinkTypes.h"
#include "ZenoCommonDataSource.generated.h"

/**
 * Utilities to access data from live link.
 */
UCLASS()
class ZENOLIVELINK_API UZenoCommonDataSource : public UObject
{
	GENERATED_BODY()

public:
	UZenoCommonDataSource();

	/**
	 * Check live link has connected to zeno
	 */
	static bool HasConnectToZeno();

	static TArray<FLiveLinkSubjectKey> GetAllSubjectKeys(bool bIncludeDisabled = false, bool bIncludeVirtual = false);
	
private:
};
