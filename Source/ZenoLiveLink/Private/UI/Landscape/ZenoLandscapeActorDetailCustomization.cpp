#include "ZenoLandscapeActorDetailCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Landscape.h"
#include "LandscapeModule.h"
#include "Interfaces/ITextureEditorModule.h"
#include "UI/Landscape/ZenoLandscapeHelper.h"
#include "UI/Landscape/ZenoLandscapeSimpleBrush.h"

#define LOCTEXT_NAMESPACE "FZenoLandscapeActorDetailCustomization"

FZenoLandscapeActorDetailCustomization::FZenoLandscapeActorDetailCustomization()
{
	UISettings = NewObject<UZenoLandscapeActorSetting>();
	UISettings->AddToRoot();
	
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;

	UISettingDetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	UISettingDetailsView->SetObject(UISettings, true);
}

FZenoLandscapeActorDetailCustomization::~FZenoLandscapeActorDetailCustomization()
{
	UISettings->RemoveFromRoot();
	UISettings->MarkAsGarbage();
}

TSharedRef<IDetailCustomization> FZenoLandscapeActorDetailCustomization::MakeInstance()
{
	return MakeShared<FZenoLandscapeActorDetailCustomization>();
}

void FZenoLandscapeActorDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> CustomizedObjects;
	DetailBuilder.GetObjectsBeingCustomized(CustomizedObjects);
	if (CustomizedObjects.Num() > 0 && CustomizedObjects[0].IsValid())
	{
		TWeakObjectPtr<ALandscape> Landscape = Cast<ALandscape>(CustomizedObjects[0]);
		const FName ZenoWeightMapCategory = "ZenoWeightMap";
		IDetailCategoryBuilder& WeightmapBuilder = DetailBuilder.EditCategory(ZenoWeightMapCategory, LOCTEXT("ZenoWeightMap", "Zeno Weightmap"));
		WeightmapBuilder.AddCustomRow(LOCTEXT("Settings", "Settings"))
		[
			UISettingDetailsView.ToSharedRef()
		];
		WeightmapBuilder.AddCustomRow(LOCTEXT("Settings", "Settings"))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("CreateLayer", "Create Layer"))
				.OnClicked_Lambda([Landscape, this] ()
				{
					if (Landscape.IsValid())
					{
						const ILandscapeModule& LandscapeModule = FModuleManager::GetModuleChecked<ILandscapeModule>("Landscape");
						const int32 SimpleLayerIndex = LandscapeModule.GetLandscapeEditorServices()->GetOrCreateEditLayer(UISettings->NewLayerName, Landscape.Get());
						FLandscapeLayer* Layer = Landscape->GetLayer(SimpleLayerIndex);
						Layer->BlendMode = ELandscapeBlendMode::LSBM_AdditiveBlend;
						Layer->HeightmapAlpha = 0;
						
						ITextureEditorModule& TextureEditor = FModuleManager::LoadModuleChecked<ITextureEditorModule>("TextureEditor");
						for (const TObjectPtr<ULandscapeComponent>& Component : Landscape->LandscapeComponents)
						{
							if (Component)
							{
								Component->ForEachLayer([Guid = Layer->Guid, &TextureEditor, UISettings = UISettings] (const FGuid& InGuid, const FLandscapeLayerComponentData& ComponentData)
								{
									if (InGuid == Guid)
									{
										UISettings->EncounteredTextures.Add(Guid, ComponentData.HeightmapData.Texture);
										TSharedRef<ITextureEditorToolkit> Editor = TextureEditor.CreateTextureEditor(EToolkitMode::Standalone, nullptr, ComponentData.HeightmapData.Texture);
										Editor->SetZoomMode(ETextureEditorZoomMode::Fill);
									}
								});
							}
						}
						UISettingDetailsView->ForceRefresh();
					}
					return FReply::Handled();
				})
			]
		];
	}

}

#undef LOCTEXT_NAMESPACE
