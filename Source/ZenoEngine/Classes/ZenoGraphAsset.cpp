#include "ZenoGraphAsset.h"

#include "Utilities/ZenoEngineTypes.h"

TOptional<const FZenoOutputParameterDescriptor> UZenoGraphAsset::FindFirstOutputParameterDescriptor(
	const EZenoSubjectType Type) const
{
	for (const auto& Descriptor : OutputParameterDescriptors)
	{
		if (Descriptor.Type == Type)
		{
			return Descriptor;
		}
	}
	return {};
}
