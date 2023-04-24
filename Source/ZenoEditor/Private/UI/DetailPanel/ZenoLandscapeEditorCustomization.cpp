#include "ZenoLandscapeEditorCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "ZenoDetailPanelService.h"
#include "Blueprint/Panel/ZenoLandscapeObject.h"

#define LOCTEXT_NAMESPACE "FZenoLandscapeEditorCustomization"

void FZenoLandscapeEditorCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	const static FName NewLandscapeLocationPropertyName = GET_MEMBER_NAME_CHECKED(UZenoLandscapeObject, NewLandscape_Location);
	const TSharedRef<IPropertyHandle> NL_LocationHandle = DetailBuilder.GetProperty(NewLandscapeLocationPropertyName);
	// Create action panel for import
	
	static FName NAME_Action = "Action";
	static FText LOC_Action = LOCTEXT("Action", "Action");
	if (DetailBuilder.IsPropertyVisible(NL_LocationHandle))
	{
		IDetailCategoryBuilder& Builder = DetailBuilder.EditCategory(NAME_Action, LOC_Action, ECategoryPriority::Uncommon);
		Builder.AddCustomRow(LOCTEXT("Apply", "Apply"), false)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("Apply", "Apply"))
			]
		];
	}
}

TSharedRef<IDetailCustomization> FZenoLandscapeEditorCustomization::Create()
{
	return MakeShared<FZenoLandscapeEditorCustomization>();
}

REGISTER_ZENO_DETAIL_CUSTOMIZATION("ZenoLandscapeObject", FZenoLandscapeEditorCustomization);

#undef LOCTEXT_NAMESPACE
