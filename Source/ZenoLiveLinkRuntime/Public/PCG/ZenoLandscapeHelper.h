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

	/**
	 * Get the height data of the landscape in the given bound
	 * @param Landscape: The landscape to get height data from
	 * @param InOutBound: The bound to get height data in **World Space**
	 * @param OutSize: The XY size of the height data
	 * @param OutSuccess: Whether the operation is successful
	 **/
	ZENOLIVELINKRUNTIME_API TArray<uint16> GetHeightDataInBound(const ALandscapeProxy* Landscape, FBox& InOutBound, FIntPoint& OutSize, bool& OutSuccess);

	/**
	 * Generate random points on the landscape with given seed and bounds
	 * @param InBound: The bound of generate points in
	 * @param Landscape: The landscape to generate points on
	 * @param NumPoints: The number of points to generate
	 * @param Seed: The seed to generate points
	 **/
	ZENOLIVELINKRUNTIME_API TArray<FVector> ScatterPoints(const ALandscapeProxy* Landscape, uint32 NumPoints, int32 Seed = 0, const FBox& InBound = FBox{ForceInit});


	FORCEINLINE int32 ComputeSeed(const int32 A)
	{
		return (A * 196314165U) + 907633515U;
	}

	FORCEINLINE int32 ComputeSeed(const int32 A, const int32 B)
	{
		return ((A * 196314165U) + 907633515U) ^ ((B * 73148459U) + 453816763U);
	}

	FORCEINLINE int32 ComputeSeed(const int32 A, const int32 B, const int32 C)
	{
		return ((A * 196314165U) + 907633515U) ^ ((B * 73148459U) + 453816763U) ^ ((C * 34731343U) + 453816743U);
	}
}
