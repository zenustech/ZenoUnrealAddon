#include "Input/ZenoInputParameter.h"

UZenoInputParameter::UZenoInputParameter()
{
	Type = EZenoParamType::Invalid;
}

UZenoInputParameter* UZenoInputParameter::CreateInputParameter(UObject* Outer, const FString& InParameterName)
{
	UZenoInputParameter* InputParameter = NewObject<UZenoInputParameter>(Outer, MakeUniqueObjectName(Outer, StaticClass(), FName(InParameterName)));
	InputParameter->Name = InParameterName;
	return InputParameter;
}

zeno::remote::ParamValue UZenoInputParameter::GatherParamValue() const
{
	checkf(false, TEXT("You must override this function in derived class"));
	return {};
}

FString UZenoInputParameter::GetParameterName() const
{
	return Name;
}
