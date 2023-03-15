#pragma once

class FZenoLandscapeVisualDataPropertyView : public IPropertyTypeCustomization
{
public:
	FZenoLandscapeVisualDataPropertyView();
	
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override;

private:
	TSharedPtr<IDetailsView> DetailsView;
	TSharedPtr<FUICommandList> CommandList;

public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
};
