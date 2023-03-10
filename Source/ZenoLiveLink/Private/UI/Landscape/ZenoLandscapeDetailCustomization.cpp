#include "ZenoLandscapeDetailCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "Selection.h"
#include "UI/Landscape/LandscapeToolZenoBridge.h"
#include "UI/Landscape/ZenoLandscapeEditorObject.h"
#include "UI/Landscape/ZenoLandscapeHelper.h"

#define LOCTEXT_NAMESPACE "FZenoLandscapeDetailCustomization"

FName FZenoLandscapeDetailCustomization_Base::GetCurrentMode() const
{
	return UZenoLandscapeTool::LandscapeToolCurrentMode;
}

FZenoLandscapeDetailCustomization::FZenoLandscapeDetailCustomization()
{
	CustomizationMap.Add(UZenoLandscapeTool::NAME_ImportHeightfield, MakeShared<FZenoLandscapeDetailCustomization_ImportLandscape>());
	CustomizationMap.Add(UZenoLandscapeTool::NAME_VisualLandscapeLayer, MakeShared<FZenoLandscapeDetailCustomization_VisualLayer>());
}

void FZenoLandscapeDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	if (CustomizationMap.Contains(GetCurrentMode()))
	{
		CustomizationMap[GetCurrentMode()]->CustomizeDetails(DetailBuilder);
	}
}

void FZenoLandscapeDetailCustomization::Register()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.RegisterCustomClassLayout("ZenoLandscapeEditorObject", FOnGetDetailCustomizationInstance::CreateStatic(&FZenoLandscapeDetailCustomization::GetInstance));
}

void FZenoLandscapeDetailCustomization::UnRegister()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.UnregisterCustomClassLayout("ZenoLandscapeEditorObject");
}

TSharedRef<IDetailCustomization> FZenoLandscapeDetailCustomization::GetInstance()
{
	return MakeShared<FZenoLandscapeDetailCustomization>();
}

void FZenoLandscapeDetailCustomization_ImportLandscape::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TSharedRef<IPropertyHandle> PropertyHandle_SubjectKey = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UZenoLandscapeEditorObject, SelectedSubjectKey));
	TSharedRef<IPropertyHandle> PropertyHandle_QuadsPerSection = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UZenoLandscapeEditorObject, ImportLandscape_QuadsPerSection));
	TSharedRef<IPropertyHandle> PropertyHandle_SectionsPerComponent = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UZenoLandscapeEditorObject, ImportLandscape_SectionsPerComponent));
	TSharedRef<IPropertyHandle> PropertyHandle_ComponentCount = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UZenoLandscapeEditorObject, ImportLandscape_ComponentCount));
	PropertyHandle_SubjectKey->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([PropertyHandle_SubjectKey, PropertyHandle_QuadsPerSection, PropertyHandle_SectionsPerComponent, PropertyHandle_ComponentCount]
	{
		FLiveLinkSubjectKey* Key;
		FLiveLinkSubjectKey*& OutPtr = Key;
		PropertyHandle_SubjectKey->GetValueData(reinterpret_cast<void*&>(OutPtr));
		if (!Key->SubjectName.IsNone())
		{
			int32 InOutQuadsPerSection, InOutSectionsPerComponent;
			FIntPoint OutComponentCount;
			FZenoLandscapeHelper::ChooseBestComponentSizeForSubject(*Key, InOutQuadsPerSection, InOutSectionsPerComponent, OutComponentCount);
			PropertyHandle_QuadsPerSection->SetValue(InOutQuadsPerSection);
			PropertyHandle_SectionsPerComponent->SetValue(InOutSectionsPerComponent);

			FIntPoint* CurrentComponentCount;
			FIntPoint*& OutComponentPtr = CurrentComponentCount;
			PropertyHandle_ComponentCount->GetValueData(reinterpret_cast<void*&>(OutComponentPtr));
			*CurrentComponentCount = OutComponentCount;
		}
	}));
}

void FZenoLandscapeDetailCustomization_VisualLayer::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TSharedRef<IPropertyHandle> PropertyHandle_LandscapeActor = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UZenoLandscapeEditorObject, LayerVisual_LandscapeActors));
	TSharedRef<IPropertyHandle> PropertyHandle_VisualData = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UZenoLandscapeEditorObject, LayerVisual_VisualData));
	IDetailCategoryBuilder& VisualCategoryBuilder = DetailBuilder.EditCategory(FName(TEXT("Visual")), LOCTEXT("Visual", "Visual"), ECategoryPriority::Important);
	VisualCategoryBuilder.AddCustomRow(FText())
	[
		SNew(SButton)
	    .Text(LOCTEXT("BindData", "Bind Data"))
	    .OnClicked(FOnClicked::CreateLambda([PropertyHandle_VisualData]
	    {
			UZenoLandscape_VisualData* DataMap = NewObject<UZenoLandscape_VisualData>();
			// UZenoLandscape_VisualData*& RefMap = DataMap;
			// PropertyHandle_VisualData->GetValueData(reinterpret_cast<void*&>(RefMap));
			PropertyHandle_VisualData->SetValue(DataMap);

			TArray<ALandscapeProxy*> SelectedLandscapes;
			USelection* SelectedActors = GEditor->GetSelectedActors();
			SelectedActors->GetSelectedObjects<ALandscapeProxy>(SelectedLandscapes);
			// ALandscapeProxy*& RefLandscapeActorOption = LandscapeActorOption;
			// PropertyHandle_LandscapeActor->GetValueData(reinterpret_cast<void*&>(RefLandscapeActorOption));

			for (ALandscapeProxy* Landscape : SelectedLandscapes)
			// if (ALandscapeProxy* Landscape = LandscapeActorOption)
			{
				if (IsValid(Landscape) && Landscape->GetFName() != NAME_None && IsValid(DataMap))
				{
					 UZenoLandscape_VisualData* VisualData = DataMap;
					 ALandscapeProxy* LandscapeActor = Landscape;
					 for (const TObjectPtr<ULandscapeComponent>& Component : LandscapeActor->LandscapeComponents)
					 {
						  if (Component)
						  {
							  Component->ForEachLayer([VisualData] (const FGuid& Guid, FLandscapeLayerComponentData& Data)
							  {
								   FZenoLandscape_VisualData_Component ComponentData;
								   ComponentData.HeightData.Texture = Data.HeightmapData.Texture;
								   ComponentData.WeightData.Textures = Data.WeightmapData.Textures;
								   ComponentData.WeightData.LayerAllocations = Data.WeightmapData.LayerAllocations;
								   ComponentData.WeightData.TextureUsages = Data.WeightmapData.TextureUsages;
								   VisualData->Components.Add(Guid, ComponentData);
							  });
						  }
					 }
				}
			}
	    	return FReply::Handled();
	    }))
	];
}

#undef LOCTEXT_NAMESPACE
