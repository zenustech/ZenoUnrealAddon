#include "ZenoLandscapeHelper.h"

#include "LandscapeImportHelper.h"
#include "LiveLinkTypes.h"
#include "Role/ZenoLiveLinkTypes.h"
#include "UI/ZenoCommonDataSource.h"

void FZenoLandscapeHelper::ExpandHeightmapData(const TArray<uint16>& InHeightMap, const FIntVector2& InTargetResolution,
                                               TArray<uint16>& OutHeightMap)
{
	check(InHeightMap != OutHeightMap);

	int32 Size = sqrt(InHeightMap.Num());
	FLandscapeImportResolution ImportResolution(Size, Size);
	FLandscapeImportResolution TargetResolution(InTargetResolution.X, InTargetResolution.Y);

	FLandscapeImportHelper::TransformHeightmapImportData(InHeightMap, OutHeightMap, ImportResolution, TargetResolution, ELandscapeImportTransformType::Resample);
}

uint16 FZenoLandscapeHelper::RemapFloatToUint16(const float InFloat)
{
	uint16 Height;
	if (InFloat > 256.f)
	{
		 Height = UINT16_MAX;
	} else if (InFloat < -256.f)
	{
		 Height = 0;
	} else
	{
		 Height = (InFloat + 255.f) / 512.f * 0xFFFF;
	}
	return Height;
}

bool FZenoLandscapeHelper::ChooseBestComponentSizeForSubject(const FLiveLinkSubjectKey& Key,
	int32& InOutQuadsPerSection, int32& InOutSectionsPerComponent, FIntPoint& OutComponentCount)
{
	if (const TOptional<FLiveLinkSubjectFrameData> FrameData = FZenoCommonDataSource::GetFrameData(Key); !Key.SubjectName.IsNone() && FrameData.IsSet())
	{
		const FLiveLinkHeightFieldStaticData* Data = FrameData->StaticData.Cast<FLiveLinkHeightFieldStaticData>();
		const int32 Size = sqrt(Data->Size);
		FLandscapeImportHelper::ChooseBestComponentSizeForImport(Size, Size, InOutQuadsPerSection, InOutSectionsPerComponent, OutComponentCount);
		return true;
	}
	return false;
}
