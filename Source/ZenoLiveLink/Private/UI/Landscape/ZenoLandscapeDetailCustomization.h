#pragma once
#include "IDetailCustomization.h"

class FZenoLandscapeDetailCustomization : public IDetailCustomization
{
public:
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
