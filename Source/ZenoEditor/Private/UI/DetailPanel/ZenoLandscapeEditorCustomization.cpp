﻿#include "ZenoLandscapeEditorCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "SlateOptMacros.h"
#include "ZenoDetailPanelService.h"
#include "Blueprint/ZenoEditorLandscapeLibrary.h"
#include "Blueprint/Panel/ZenoLandscapeObject.h"
#include "Client/ZenoLiveLinkClientSubsystem.h"
#include "UObject/GCObjectScopeGuard.h"

#define LOCTEXT_NAMESPACE "FZenoLandscapeEditorCustomization"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void FZenoLandscapeEditorCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	ensure(Objects.Num() == 1);
	UZenoLandscapeObject* Object = Cast<UZenoLandscapeObject>(Objects[0].Get());
	ensure(Object != nullptr);
	// SubjectKey handle
	const static FName SubjectKeyPropertyName = GET_MEMBER_NAME_CHECKED(UZenoLandscapeObject, SelectedSubjectKey);
	const TSharedRef<IPropertyHandle> SubjectKeyHandle = DetailBuilder.GetProperty(SubjectKeyPropertyName);
	// Location handle
	const static FName NewLandscapeLocationPropertyName = GET_MEMBER_NAME_CHECKED(UZenoLandscapeObject, NewLandscape_Location);
	const TSharedRef<IPropertyHandle> NL_LocationHandle = DetailBuilder.GetProperty(NewLandscapeLocationPropertyName);
	
	// Create action panel for import
	const static FName NAME_Action = "Action";
	const static FText LOC_Action = LOCTEXT("Action", "Action");
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
				.OnClicked_Static(&FZenoLandscapeEditorCustomization::ApplyLandscapeImport, Object)
			]
		];
		
		// Watch subject key change
		{
			TSharedRef<IPropertyHandle> QuadsPerSectionHandle = DetailBuilder.GetProperty(
				GET_MEMBER_NAME_CHECKED(UZenoLandscapeObject, ImportLandscape_QuadsPerSection));
			TSharedRef<IPropertyHandle> SectionsPerComponentHandle = DetailBuilder.GetProperty(
				GET_MEMBER_NAME_CHECKED(UZenoLandscapeObject, ImportLandscape_SectionsPerComponent));
			TSharedRef<IPropertyHandle> ComponentCountHandle = DetailBuilder.GetProperty(
				GET_MEMBER_NAME_CHECKED(UZenoLandscapeObject, ImportLandscape_ComponentCount));
			
			SubjectKeyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda(
			[SubjectKeyHandle, QuadsPerSectionHandle, SectionsPerComponentHandle, ComponentCountHandle]
			{
				FLiveLinkSubjectKey* Key;
				FLiveLinkSubjectKey*& OutRef = Key;
				SubjectKeyHandle->GetValueData(reinterpret_cast<void*&>(OutRef));
				if (!Key->SubjectName.IsNone())
				{
					int32 InOutQuadsPerSection;
					QuadsPerSectionHandle->GetValue(InOutQuadsPerSection);
					int32 InOutSectionsPerComponent;
					SectionsPerComponentHandle->GetValue(InOutSectionsPerComponent);
					UZenoEditorLandscapeLibrary::ChooseBestComponentSizeForSubject(
						*Key, InOutQuadsPerSection, InOutSectionsPerComponent
					)->GetFuture()
					.Then(
					[QuadsPerSectionHandle, SectionsPerComponentHandle, ComponentCountHandle]
					(const TResultFuture<TTuple<int32, int32, FIntPoint>>& Result)
					{
						if (Result.IsReady() && Result.Get().IsSet())
						{
							auto Data = Result.Get().GetValue();
							QuadsPerSectionHandle->SetValue(Data.Get<0>());
							SectionsPerComponentHandle->SetValue(Data.Get<1>());
							FIntPoint* CurrentComponentCount;
							FIntPoint*& OutComponentPtr = CurrentComponentCount;
							ComponentCountHandle->GetValueData(reinterpret_cast<void*&>(OutComponentPtr));
							*CurrentComponentCount = Data.Get<2>();
						}
					});
				}
			}));
		}
	}

}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FReply FZenoLandscapeEditorCustomization::ApplyLandscapeImport(UZenoLandscapeObject* Object)
{
	if (IsValid(Object))
	{
		auto Key = Object->SelectedSubjectKey;
		UZenoLiveLinkClientSubsystem* Subsystem = GEngine->GetEngineSubsystem<UZenoLiveLinkClientSubsystem>();
		const TSharedPromise<zeno::remote::HeightField> HeightfieldPromise = Subsystem->TryLoadSubjectRemotely<
			zeno::remote::HeightField>(Key.SubjectName);
		HeightfieldPromise->GetFuture().Then([Object](const TResultFuture<zeno::remote::HeightField>& Result)
		{
			if (!IsValid(Object) || !Result.IsReady() || !Result.Get().IsSet()) {return;}
			FGCObjectScopeGuard Guard { Object };

			zeno::remote::HeightField HeightField = Result.Get().GetValue();
			
			const int32 SizeX = Object->ImportLandscape_ComponentCount.X * Object->ImportLandscape_QuadsPerSection + 1;
			const int32 SizeY = Object->ImportLandscape_ComponentCount.Y * Object->ImportLandscape_QuadsPerSection + 1;

			// Expand height map to suitable size
			TArray<uint16> HeightData;
			TArray<uint16> ExpandedHeightData;
			HeightData.Reserve(HeightField.Nx * HeightField.Ny);
			for (const std::vector<uint16>& Row : HeightField.Data)
			{
				for (uint16 Value : Row)
				{
					HeightData.Emplace(Value);
				}
			}
			UZenoEditorLandscapeLibrary::ExpandHeightmapData(HeightData, { SizeX, SizeY }, ExpandedHeightData);

			TMap<FGuid, TArray<uint16>> HeightDataPerLayers;
			TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayers;
			TArray<FLandscapeImportLayerInfo> LandscapeImportLayerInfos;

			HeightDataPerLayers.Add({FGuid(), ExpandedHeightData});
		    MaterialLayerDataPerLayers.Add({FGuid(), LandscapeImportLayerInfos});

			FScopedTransaction Transaction(LOCTEXT("Undo", "Creating New Landscape"));

			ALandscape* Landscape = GWorld->SpawnActor<ALandscape>(Object->NewLandscape_Location,
			                                                       Object->NewLandscape_Rotation);
			Landscape->bCanHaveLayersContent = Object->bImportLandscape_CanHaveLayer;
			Landscape->LandscapeMaterial = Object->ImportLandscape_MaterialInterface.Get(false);
			Landscape->SetActorRelativeScale3D(Object->ImportLandscape_Scale);
			Landscape->StaticLightingLOD = FMath::DivideAndRoundUp(
				FMath::CeilLogTwo((SizeX * SizeY) / (2048 * 2048) + 1),
				static_cast<uint32>(2));

			Landscape->Import(FGuid::NewGuid(), 0, 0, SizeX - 1, SizeY - 1,
			                  Object->ImportLandscape_SectionsPerComponent,
			                  Object->ImportLandscape_QuadsPerSection, HeightDataPerLayers,
			                  *FString::Printf(
				                  TEXT("zenor://%s"), *Object->SelectedSubjectKey.SubjectName.ToString()),
			                  MaterialLayerDataPerLayers, ELandscapeImportAlphamapType::Additive);

			ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();
			check(LandscapeInfo);

			FActorLabelUtilities::SetActorLabelUnique(Landscape, ALandscape::StaticClass()->GetName());

			LandscapeInfo->UpdateLayerInfoMap(Landscape);
		});
	}
	return FReply::Handled();
}

TSharedRef<IDetailCustomization> FZenoLandscapeEditorCustomization::Create()
{
	return MakeShared<FZenoLandscapeEditorCustomization>();
}

REGISTER_ZENO_DETAIL_CUSTOMIZATION("ZenoLandscapeObject", FZenoLandscapeEditorCustomization);

#undef LOCTEXT_NAMESPACE
