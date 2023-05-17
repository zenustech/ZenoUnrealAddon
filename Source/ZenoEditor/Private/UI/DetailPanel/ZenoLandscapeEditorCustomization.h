#pragma once
#include "IDetailCustomization.h"

class FZenoLandscapeEditorCustomization : public IDetailCustomization
{
public:
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	static FReply ApplyLandscapeImport(class UZenoLandscapeObject* Object);

	static TSharedRef<IDetailCustomization> Create();
};
