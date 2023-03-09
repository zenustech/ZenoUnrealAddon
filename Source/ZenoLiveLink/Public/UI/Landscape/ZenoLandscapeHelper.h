#pragma once

struct FLiveLinkSubjectKey;

class ZENOLIVELINK_API FZenoLandscapeHelper
{
public:
	static void ExpandHeightmapData(const TArray<uint16>& InHeightMap, const FIntVector2& InTargetResolution, TArray<uint16>& OutHeightMap);

	static uint16 RemapFloatToUint16(const float InFloat);

	static bool ChooseBestComponentSizeForSubject(const FLiveLinkSubjectKey& Key, int32& InOutQuadsPerSection, int32& InOutSectionsPerComponent, FIntPoint& OutComponentCount);
	
};
