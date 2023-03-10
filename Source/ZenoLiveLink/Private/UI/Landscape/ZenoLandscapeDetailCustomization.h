#pragma once
#include "IDetailCustomization.h"

class FZenoLandscapeDetailCustomization_Base : public IDetailCustomization
{
protected:
	FName GetCurrentMode() const;
};

class FZenoLandscapeDetailCustomization : public FZenoLandscapeDetailCustomization_Base
{
public:
	FZenoLandscapeDetailCustomization();
	
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	static void Register();
	static void UnRegister();

	static TSharedRef<IDetailCustomization> GetInstance();

	TMap<FName, TSharedRef<IDetailCustomization>> CustomizationMap;
};

class FZenoLandscapeDetailCustomization_ImportLandscape : public FZenoLandscapeDetailCustomization_Base
{
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};

class FZenoLandscapeDetailCustomization_VisualLayer : public FZenoLandscapeDetailCustomization_Base
{
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
