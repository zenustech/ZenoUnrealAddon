// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZenoInputParameter.h"
#include "ZenoInputInteger.generated.h"

UCLASS(DefaultToInstanced)
class ZENOENGINE_API UZenoInputInteger : public UZenoInputParameter
{
	GENERATED_BODY()

public:
	explicit UZenoInputInteger();
	static UZenoInputInteger* CreateInputParameter(UObject* Outer, const FString& InParameterName = "DefaultParam", const int32 InValue = 0);
	
	virtual zeno::remote::ParamValue GatherParamValue() const override;

protected:
	UPROPERTY(EditAnywhere, Category = Zeno, DisplayName = "Value")
	int32 Value = 0;
};
