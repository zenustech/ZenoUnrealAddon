#pragma once

class ZENOLIVELINK_API FZenoLandscapeHelper
{
public:
	static void ExpandHeightmapData(const TArray<uint16>& InHeightMap, const FIntVector2& InTargetResolution, TArray<uint16>& OutHeightMap);
	
};
