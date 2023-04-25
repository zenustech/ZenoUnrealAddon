#include "ZenoLandscapeEditorCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "ZenoDetailPanelService.h"
#include "Blueprint/Panel/ZenoLandscapeObject.h"
#include "Client/ZenoLiveLinkClientSubsystem.h"

#define LOCTEXT_NAMESPACE "FZenoLandscapeEditorCustomization"

void FZenoLandscapeEditorCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	ensure(Objects.Num() == 1);
	UZenoLandscapeObject* Object = Cast<UZenoLandscapeObject>(Objects[0].Get());
	ensure(Object != nullptr);
	// Location handle
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
				.OnClicked_Lambda([Object]
				{
					if (IsValid(Object))
					{
						auto Key = Object->SelectedSubjectKey;
						UZenoLiveLinkClientSubsystem* Subsystem = GEngine->GetEngineSubsystem<UZenoLiveLinkClientSubsystem>();
						TSharedPromise<zeno::remote::Mesh> Mesh = Subsystem->TryLoadSubjectRemotely<zeno::remote::Mesh>(Key.SubjectName);
						Mesh->GetFuture().Then([] (TResultFuture<zeno::remote::Mesh> Result)
						{
							  UE_LOG(LogTemp, Error, TEXT("Is Valid: %d"), Result.IsReady());
						});
					}
					return FReply::Handled();
				})
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
