#pragma once

enum ETextureRenderTargetFormat : int;
struct FLiveLinkSubjectKey;

class ZENOLIVELINK_API FZenoLandscapeHelper
{
public:
	static void ExpandHeightmapData(const TArray<uint16>& InHeightMap, const FIntVector2& InTargetResolution, TArray<uint16>& OutHeightMap);

	static uint16 RemapFloatToUint16(const float InFloat);

	static float RemapUint16ToFloat(const uint16 InInt);

	static bool ChooseBestComponentSizeForSubject(const FLiveLinkSubjectKey& Key, int32& InOutQuadsPerSection, int32& InOutSectionsPerComponent, FIntPoint& OutComponentCount);

	static UTextureRenderTarget2D* GetOrCreateTransientRenderTarget2D(UTextureRenderTarget2D* InRenderTarget, FName InRenderTargetName, const FIntPoint& InSize, ETextureRenderTargetFormat InFormat, const FLinearColor& InClearColor = FLinearColor::Black, bool bInAutoGenerateMipMaps = false);

	static UMaterialInstanceDynamic* GetOrCreateTransientMid(UMaterialInstanceDynamic* InMID, FName InMIDName, UMaterialInterface* InMaterialInterface, EObjectFlags InAdditionalObjectFlags = RF_NoFlags);
	
};
