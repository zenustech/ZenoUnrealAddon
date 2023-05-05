#include "Utilities/ZenoEngineTypes.h"

#include <zeno/unreal/ZenoRemoteTypes.h>

FZenoInputParameterDescriptor FZenoInputParameterDescriptor::FromZenoType(
	const zeno::remote::ParamDescriptor& Descriptor)
{
	return {
		Descriptor.Name.c_str(),
		static_cast<EZenoParamType>(Descriptor.Type),
	};
}

FZenoOutputParameterDescriptor FZenoOutputParameterDescriptor::FromZenoType(
	const zeno::remote::ParamDescriptor& Descriptor)
{
	return {
		Descriptor.Name.c_str(),
		static_cast<EZenoSubjectType>(Descriptor.Type),
	};
}
