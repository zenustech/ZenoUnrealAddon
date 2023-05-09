#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Utilities/ZenoEngineTypes.h"
#include "ZenoInputParameter.generated.h"

/**
 *
 */
UCLASS(DefaultToInstanced)
class ZENOENGINE_API UZenoInputParameter : public UObject
{
	GENERATED_BODY()

public:
	explicit UZenoInputParameter();

	static UZenoInputParameter* CreateInputParameter(UObject* Outer, const FString& InParameterName = "DefaultParam");

	virtual struct zeno::remote::ParamValue GatherParamValue() const;

protected:
	/** Parameter name */
	UPROPERTY(VisibleAnywhere, Category = Zeno, DisplayName = "Param Name", AdvancedDisplay)
	FString Name;

	/** Parameter type */
	UPROPERTY(VisibleAnywhere, Category = Zeno, DisplayName = "Param Type", AdvancedDisplay)
	EZenoParamType Type;

public:
	/** Get parameter name */
	FORCEINLINE UE_NODISCARD FString GetParameterName() const;

protected:
};
