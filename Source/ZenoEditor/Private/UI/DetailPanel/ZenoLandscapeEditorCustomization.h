#pragma once
#include "IDetailCustomization.h"

class FZenoLandscapeEditorCustomization : public IDetailCustomization
{
public:
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	static TSharedRef<IDetailCustomization> Create();
};
