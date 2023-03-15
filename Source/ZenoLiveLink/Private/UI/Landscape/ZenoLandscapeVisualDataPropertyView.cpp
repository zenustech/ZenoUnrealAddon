#include "ZenoLandscapeVisualDataPropertyView.h"

#include "DetailWidgetRow.h"
#include "PropertyCustomizationHelpers.h"
#include "Command/ZenoTextureExportCommand.h"
#include "UI/Landscape/ZenoLandscapeEditorObject.h"

class FZenoLandscapeTexture2DExportPropertyView : public IPropertyTypeCustomization
{

public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance()
	{
		return MakeShared<FZenoLandscapeTexture2DExportPropertyView>();
	}

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override
	{
		HeaderRow.NameContent()
		[
			SNew(STextBlock).Text(FText::FromString("123"))
		];
	}
	
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override
	{
	}
};

FZenoLandscapeVisualDataPropertyView::FZenoLandscapeVisualDataPropertyView()
{
	CommandList = MakeShared<FUICommandList>();

	CommandList->MapAction(FZenoTextureExportCommand::Get().ExportTexture, FExecuteAction::CreateLambda([]
	{
		UE_LOG(LogTemp, Warning, TEXT("123"));
	}));

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.bAllowSearch = true;
	DetailsViewArgs.HostCommandList = CommandList;
	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	// UZenoLandscape_VisualData const* VisualDataCDO = Cast<UZenoLandscape_VisualData>(UZenoLandscape_VisualData::StaticClass()->GetDefaultObject());
	DetailsView->RegisterInstancedCustomPropertyTypeLayout("WeakObjectProperty", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FZenoLandscapeTexture2DExportPropertyView::MakeInstance));
	DetailsView->SetObject(UZenoLandscape_VisualData::StaticClass()->GetDefaultObject());
}

void FZenoLandscapeVisualDataPropertyView::CustomizeHeader(
	TSharedRef<IPropertyHandle> PropertyHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{

	PropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([View = DetailsView.ToWeakPtr(), PropertyHandle]
	{
		if (View.IsValid())
		{
			UObject* Object;
			PropertyHandle->GetValue(Object);
			UZenoLandscape_VisualData* VisualDataObject = Cast<UZenoLandscape_VisualData>(Object);
			View.Pin()->SetObject(VisualDataObject);
		}
	}));

	HeaderRow
	.NameContent()
	[
		SNew(STextBlock).Text(PropertyHandle->GetPropertyDisplayName()).Font(CustomizationUtils.GetRegularFont())
	]
	.ValueContent()
	[
		SNew(SVerticalBox)
		// + SVerticalBox::Slot()
		// .AutoHeight()
		// [
		// 	SNew(SProperty, PropertyHandle)
		// 	.ShouldDisplayName(false)
		// ]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			DetailsView.ToSharedRef()
		]
	];
}

void FZenoLandscapeVisualDataPropertyView::CustomizeChildren(
	TSharedRef<IPropertyHandle> PropertyHandle,
	IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
}

TSharedRef<IPropertyTypeCustomization> FZenoLandscapeVisualDataPropertyView::MakeInstance()
{
	return MakeShared<FZenoLandscapeVisualDataPropertyView>();
}
