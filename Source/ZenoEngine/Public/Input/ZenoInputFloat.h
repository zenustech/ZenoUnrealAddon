// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZenoInputParameter.h"

#include "ZenoInputFloat.generated.h"

/**
 * 
 */
UCLASS(DefaultToInstanced)
class ZENOENGINE_API UZenoInputFloat : public UZenoInputParameter
{
	GENERATED_BODY()

public:
	explicit UZenoInputFloat();
	static UZenoInputFloat* CreateInputParameter(UObject* Outer, const FString& InParameterName = "DefaultParam", const float InValue = .0f);
	virtual zeno::remote::ParamValue GatherParamValue() const override;

protected:
	UPROPERTY(EditAnywhere, Category = Zeno, DisplayName = "Value")
	float Value = .0f;
};
