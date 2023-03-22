#include "ZenoLandscapeHelper.h"

#include "LandscapeImportHelper.h"
#include "LiveLinkTypes.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInstanceDynamic.h"
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

float FZenoLandscapeHelper::RemapUint16ToFloat(const uint16 InInt)
{
	return static_cast<float>(InInt) / 0xFFFF * 512.f - 255.f;
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

UTextureRenderTarget2D* FZenoLandscapeHelper::GetOrCreateTransientRenderTarget2D(UTextureRenderTarget2D* InRenderTarget,
	FName InRenderTargetName, const FIntPoint& InSize, ETextureRenderTargetFormat InFormat,
	const FLinearColor& InClearColor, bool bInAutoGenerateMipMaps)
{
	const EPixelFormat PixelFormat = GetPixelFormatFromRenderTargetFormat(InFormat);
	if (InSize.X <= 0 ||
		InSize.Y <= 0 ||
		PixelFormat == EPixelFormat::PF_Unknown)
	{
		return nullptr;
	}

	if (IsValid(InRenderTarget))
	{
		if (InRenderTarget->SizeX == InSize.X &&
			InRenderTarget->SizeY == InSize.Y &&
			InRenderTarget->GetFormat() == PixelFormat &&
			InRenderTarget->ClearColor == InClearColor &&
			InRenderTarget->bAutoGenerateMips == bInAutoGenerateMipMaps)
		{
			return InRenderTarget;
		}
	}

	UTextureRenderTarget2D* NewRenderTarget2D = NewObject<UTextureRenderTarget2D>(GetTransientPackage(), MakeUniqueObjectName(GetTransientPackage(), UTextureRenderTarget2D::StaticClass(), InRenderTargetName));
	check(NewRenderTarget2D);
	NewRenderTarget2D->RenderTargetFormat = InFormat;
	NewRenderTarget2D->ClearColor = InClearColor;
	NewRenderTarget2D->bAutoGenerateMips = bInAutoGenerateMipMaps;
	NewRenderTarget2D->InitAutoFormat(InSize.X, InSize.Y);
	NewRenderTarget2D->UpdateResourceImmediate(true);

	ENQUEUE_RENDER_COMMAND(FlushRHIThreadToUpdateTextureRenderTargetReference)(
	[](FRHICommandListImmediate& RHICmdList)
	{
		RHICmdList.ImmediateFlush(EImmediateFlushType::FlushRHIThread);
	});

	return NewRenderTarget2D;
}

UMaterialInstanceDynamic* FZenoLandscapeHelper::GetOrCreateTransientMid(UMaterialInstanceDynamic* InMID,
	FName InMIDName, UMaterialInterface* InMaterialInterface, EObjectFlags InAdditionalObjectFlags)
{
	if (!IsValid(InMaterialInterface))
	{
		return nullptr;
	}

	UMaterialInstanceDynamic* ResultMID = InMID;

	if (!IsValid(InMID) || InMID->Parent != InMaterialInterface)
	{
		ResultMID = Cast<UMaterialInstanceDynamic>(InMaterialInterface);

		if (nullptr != ResultMID)
		{
			ensure(EnumHasAllFlags(InMaterialInterface->GetFlags(), EObjectFlags::RF_Transient));
		}
		else
		{
			ResultMID = UMaterialInstanceDynamic::Create(InMaterialInterface, nullptr, MakeUniqueObjectName(GetTransientPackage(), UMaterialInstanceDynamic::StaticClass(), InMIDName));
			ResultMID->SetFlags(InAdditionalObjectFlags);
		}
	}

	check(nullptr != ResultMID);
	return ResultMID;
}
