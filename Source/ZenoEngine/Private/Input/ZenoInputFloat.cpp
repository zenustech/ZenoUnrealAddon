#include "Input/ZenoInputFloat.h"
#include <string>

UZenoInputFloat::UZenoInputFloat()
{
	Type = EZenoParamType::Float;
}

UZenoInputFloat* UZenoInputFloat::CreateInputParameter(UObject* Outer, const FString& InParameterName,
	const float InValue)
{
	UZenoInputFloat* InputParameter = NewObject<UZenoInputFloat>(Outer, MakeUniqueObjectName(Outer, StaticClass(), FName(InParameterName)));
	InputParameter->Name = InParameterName;
	InputParameter->Value = InValue;
	return InputParameter;
}

zeno::remote::ParamValue UZenoInputFloat::GatherParamValue() const
{
	return { TCHAR_TO_ANSI(*GetParameterName()), static_cast<int16_t>(zeno::remote::EParamType::Float), std::to_string( Value ) };
}
