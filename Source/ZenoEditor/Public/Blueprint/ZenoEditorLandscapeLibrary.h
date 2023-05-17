#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Client/ZenoHttpClient.h"
#include "ZenoEditorLandscapeLibrary.generated.h"

UCLASS()
class ZENOEDITOR_API UZenoEditorLandscapeLibrary : public UObject
{
	GENERATED_BODY()

public:
	static void ExpandHeightmapData(const TArray<uint16>& InHeightMap, const FIntVector2& InTargetResolution,
	                                TArray<uint16>& OutHeightMap);

	static TSharedPromise<TTuple<int32, int32, FIntPoint>> ChooseBestComponentSizeForSubject(const FLiveLinkSubjectKey& Key, const int32 InQuadsPerSection, const int32 InSectionsPerComponent);
};
