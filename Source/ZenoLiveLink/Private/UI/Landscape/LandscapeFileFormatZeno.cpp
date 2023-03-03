#include "LandscapeFileFormatZeno.h"

#include "ZenoLiveLinkSource.h"
#include "Role/ZenoLiveLinkTypes.h"
#include "UI/ZenoCommonDataSource.h"

#define LOCTEXT_NAMESPACE "ZenoLandscapeEditor.NewLandscape"

FLandscapeHeightmapFileFormatZeno_Virtual::FLandscapeHeightmapFileFormatZeno_Virtual()
{
	FileTypeInfo.bSupportsExport = false;
	FileTypeInfo.Extensions.Add(".virtual_zeno_heightmap");
	FileTypeInfo.Description = LOCTEXT("FileFormatZenoVirtual_HeightmapDesc", "Heightmap .virtual_zeno_heightmap files");
}

const FLandscapeFileTypeInfo& FLandscapeHeightmapFileFormatZeno_Virtual::GetInfo() const
{
	return FileTypeInfo;
}

FLandscapeFileInfo FLandscapeHeightmapFileFormatZeno_Virtual::Validate(const TCHAR* HeightmapFilename,
	FName LayerName) const
{
	FLandscapeFileInfo Result;
	Result.DataScale = FVector { 64.0, 64.0, 64.0 / 128.0 };

	if (!UZenoCommonDataSource::HasConnectToZeno())
	{
		Result.ResultCode = ELandscapeImportResult::Error;
		Result.ErrorMessage = LOCTEXT("NotConnected", "LiveLink hasn't connected to zeno.");
		return Result;
	}
	
	FString Filename = HeightmapFilename;
	// if (!Filename.RemoveFromStart("zeno://") || !Filename.RemoveFromEnd(".virtual_zeno_heightmap"))
	if (!Filename.RemoveFromEnd(".virtual_zeno_heightmap"))
	{
		Result.ResultCode = ELandscapeImportResult::Error;
		Result.ErrorMessage = LOCTEXT("BadVirutalFileUri", "Bad zeno virtual file URI.");
	} else
	{
		int32 Index;
		Filename.FindLastChar(L'/', Index);
		Filename = Filename.Mid(Index+1);
		const FName SubjectName = FName(Filename);
		if (!FZenoLiveLinkSource::CurrentProviderInstance->HasSubject(SubjectName))
		{
			Result.ResultCode = ELandscapeImportResult::Error;
			Result.ErrorMessage = LOCTEXT("SubjectNotFound", "Subject not found.");
		} else
		{
			TOptional<FLiveLinkSubjectFrameData> FrameData = UZenoCommonDataSource::GetFrameData( { FZenoLiveLinkSource::CurrentProviderInstance->GetGuid(), SubjectName});
			if (!FrameData.IsSet() || !FrameData->StaticData.IsValid() || FrameData->StaticData.Cast<FLiveLinkHeightFieldStaticData>() == nullptr)
			{
				Result.ResultCode = ELandscapeImportResult::Error;
				Result.ErrorMessage = LOCTEXT("CouldNotFetchData", "Could not fetch data from live link.");
			} else
			{
				const FLiveLinkHeightFieldStaticData* StaticData = FrameData->StaticData.Cast<FLiveLinkHeightFieldStaticData>();
				const uint32 Size = sqrt(StaticData->Size);
				Result.PossibleResolutions.Add( { Size, Size } );
			}
		}
	}

	return Result;
}

FLandscapeImportData<uint16> FLandscapeHeightmapFileFormatZeno_Virtual::Import(const TCHAR* HeightmapFilename,
	FName LayerName, FLandscapeFileResolution ExpectedResolution) const
{
	FLandscapeImportData<uint16> ImportData;
	
	FString Filename = HeightmapFilename;
	int32 Index;
	Filename.FindLastChar(L'/', Index);
	Filename = Filename.Mid(Index+1);
	Filename.RemoveFromEnd(".virtual_zeno_heightmap");
	const FName SubjectName = FName(Filename);
	TOptional<FLiveLinkSubjectFrameData> FrameData = UZenoCommonDataSource::GetFrameData( { FZenoLiveLinkSource::CurrentProviderInstance->GetGuid(), SubjectName});
	const FLiveLinkHeightFieldStaticData* StaticData = FrameData->StaticData.Cast<FLiveLinkHeightFieldStaticData>();
	ImportData.Data.Reserve(StaticData->Data.Num());
	for (size_t Idx = 0; Idx < StaticData->Data.Num(); ++Idx)
	{
		const float Height = (StaticData->Data[Idx] + 255.f) / 512.f * 0xFFFF * 0.75;
		ImportData.Data.Push(static_cast<uint16>(Height));
	}
	ImportData.ResultCode = ELandscapeImportResult::Success;

	return ImportData;
}

void FLandscapeHeightmapFileFormatZeno_Virtual::Export(const TCHAR* HeightmapFilename, FName LayerName,
	TArrayView<const uint16> Data, FLandscapeFileResolution DataResolution, FVector Scale) const
{
	ILandscapeFileFormat<unsigned short>::Export(HeightmapFilename, LayerName, Data, DataResolution, Scale);
}

#undef LOCTEXT_NAMESPACE
