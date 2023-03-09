#include "ZenoLandscapeHelper.h"

#include "LandscapeImportHelper.h"

void FZenoLandscapeHelper::ExpandHeightmapData(const TArray<uint16>& InHeightMap, const FIntVector2& InTargetResolution,
                                               TArray<uint16>& OutHeightMap)
{
	check(InHeightMap != OutHeightMap);

	int32 Size = sqrt(InHeightMap.Num());
	FLandscapeImportResolution ImportResolution(Size, Size);
	FLandscapeImportResolution TargetResolution(InTargetResolution.X, InTargetResolution.Y);

	FLandscapeImportHelper::TransformHeightmapImportData(InHeightMap, OutHeightMap, ImportResolution, TargetResolution, ELandscapeImportTransformType::ExpandCentered);
}
