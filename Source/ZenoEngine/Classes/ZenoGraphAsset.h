#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ZenoGraphAsset.generated.h"

enum class EZenoSubjectType : int16;
struct FZenoOutputParameterDescriptor;
struct FZenoInputParameterDescriptor;

UCLASS(BlueprintType)
class ZENOENGINE_API UZenoGraphAsset : public UObject
{
	GENERATED_BODY()

public:
	/** Raw JSON string exporting by zeno export (default hotkey: ctrl+shift+e.) */
	UPROPERTY(VisibleAnywhere, Category = "Zeno", DisplayName = "Graph JSON", AdvancedDisplay)
	FString ZenoActionRecordExportedData;

	/** Input parameter descriptors. */
	UPROPERTY(VisibleAnywhere, Category = "Zeno", DisplayName = "Input Parameter Descriptors")
	TArray<FZenoInputParameterDescriptor> InputParameterDescriptors;

	/** Output parameter descriptors. */
	UPROPERTY(VisibleAnywhere, Category = "Zeno", DisplayName = "Output Parameter Descriptors")
	TArray<FZenoOutputParameterDescriptor> OutputParameterDescriptors;

	/** File path using to reimport from. */
	UPROPERTY(EditAnywhere, Category = "Zeno", DisplayName = "Reimport Path", AdvancedDisplay)
	FString ReimportPath;

	/** Find first output parameter descriptor with the given type. */
	TOptional<const FZenoOutputParameterDescriptor> FindFirstOutputParameterDescriptor(const EZenoSubjectType Type) const;
};
