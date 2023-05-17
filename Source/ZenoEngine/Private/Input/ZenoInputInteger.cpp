// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/ZenoInputInteger.h"

UZenoInputInteger::UZenoInputInteger()
{
	Type = EZenoParamType::Integer;
}

UZenoInputInteger* UZenoInputInteger::CreateInputParameter(UObject* Outer, const FString& InParameterName, const int32 InValue)
{
	UZenoInputInteger* InputParameter = NewObject<UZenoInputInteger>(Outer, MakeUniqueObjectName(Outer, StaticClass(), FName(InParameterName)));
	InputParameter->Name = InParameterName;
	InputParameter->Value = InValue;
	return InputParameter;
}

zeno::remote::ParamValue UZenoInputInteger::GatherParamValue() const
{
	return { TCHAR_TO_ANSI(*GetParameterName()), static_cast<int16_t>(zeno::remote::EParamType::Integer), std::to_string( Value ) };
}
