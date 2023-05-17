#pragma once

#include "Math/Box.h"

class UZenoPCGVolumeComponent;
class ALandscapeProxy;
class UWorld;
class AActor;

namespace Zeno::Helper
{
	/** Using same tags with the default PCG system */
	const FName DefaultPCGTag = TEXT("PCG Generated Component");
	const FName DefaultPCGDebugTag = TEXT("PCG Generated Debug Component");
	const FName DefaultPCGActorTag = TEXT("PCG Generated Actor");
	const FName MarkedForCleanupPCGTag = TEXT("PCG Marked For Cleanup");
	
	ZENOLIVELINKRUNTIME_API bool IsRuntimeOrPIE();

	ZENOLIVELINKRUNTIME_API FBox GetGridBounds(const AActor* Actor, const UZenoPCGVolumeComponent* Component);
	
	ZENOLIVELINKRUNTIME_API FBox GetActorBounds(const AActor* InActor, bool bIgnorePCGCreatedComponent = true);
	ZENOLIVELINKRUNTIME_API FBox GetLandscapeBounds(const ALandscapeProxy* InLandscapeProxy);
	
	ZENOLIVELINKRUNTIME_API TArray<TWeakObjectPtr<ALandscapeProxy>> GetLandscapeProxies(const UWorld* InWorld, const FBox& InBounds);
}
