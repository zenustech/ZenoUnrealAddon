#include "Utilities/ZenoEngineTypes.h"

#include "Input/ZenoInputFloat.h"
#include "Input/ZenoInputInteger.h"
#include "Input/ZenoInputParameter.h"

FZenoInputParameterDescriptor FZenoInputParameterDescriptor::FromZenoType(
	const zeno::remote::ParamDescriptor& Descriptor)
{
	return {
		Descriptor.Name.c_str(),
		static_cast<EZenoParamType>(Descriptor.Type),
	};
}

UZenoInputParameter* FZenoInputParameterDescriptor::CreateInputParameter(UObject* Outer) const
{
	if (Type == EZenoParamType::Float)
	{
		return UZenoInputFloat::CreateInputParameter(Outer, Name, .0f);
	}
	if (Type == EZenoParamType::Integer)
	{
		return UZenoInputInteger::CreateInputParameter(Outer, Name, 0);
	}
	return nullptr;
}
