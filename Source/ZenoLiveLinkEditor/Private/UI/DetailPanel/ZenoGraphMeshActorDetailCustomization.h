#pragma once
#include "IDetailCustomization.h"

class FZenoGraphMeshActorDetailCustomization : public IDetailCustomization
{
public:
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	static TSharedRef<IDetailCustomization> Create();
};
