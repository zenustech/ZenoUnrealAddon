#pragma once

#include "ZenoLandscapeVisualDataPropertyView.generated.h"

UENUM()
enum class ETextureExportType : uint8
{
	B8G8R8A8 = 0,
	HeightField,

	Max,
};

UCLASS()
class UZenoTextureExportSetting : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Settings", DisplayName = "Subject Name", NonTransactional)
	FString SubjectName;
	
	UPROPERTY(EditAnywhere, Category = "Settings", DisplayName = "Texture Type", NonTransactional)
	ETextureExportType Type;
};

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
