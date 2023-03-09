#include "ZenoLandscapeDetailCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "UI/Landscape/ZenoLandscapeEditorObject.h"
#include "UI/Landscape/ZenoLandscapeHelper.h"

void FZenoLandscapeDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
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
