﻿#include "UI/DetailPanel/ZenoPCGActorDetailCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "LandscapeComponent.h"
#include "Client/ZenoLiveLinkClientSubsystem.h"
#include "PCG/ZenoPCGVolume.h"
#include "PCG/ZenoPCGVolumeComponent.h"
#include "UI/DetailPanel/ZenoDetailPanelService.h"

#define LOCTEXT_NAMESPACE "FZenoPCGActorDetailCustomization"

void FZenoPCGActorDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	const FName ZenoCategoryName("Zeno");
	IDetailCategoryBuilder& ZenoCategory = DetailBuilder.EditCategory(ZenoCategoryName);

	FDetailWidgetRow& NewRow = ZenoCategory.AddCustomRow(FText::GetEmpty());

	AZenoPCGVolume* Volume;
	{
		TArray<TWeakObjectPtr<UObject>> Objects;
		DetailBuilder.GetObjectsBeingCustomized(Objects);
		check(!Objects.IsEmpty());
		Volume = Cast<AZenoPCGVolume>(Objects[0]);
	}
	
	NewRow.ValueContent()
		.MaxDesiredWidth(120.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.0f)
			[
				SNew(SButton)
				.OnClicked_Lambda([Volume] ()
				{
					UZenoLiveLinkClientSubsystem* Client = GEngine->GetEngineSubsystem<UZenoLiveLinkClientSubsystem>();
					const auto HeightFieldPtr = Volume->PCGComponent->GetLandscapeHeightData();
					if (!HeightFieldPtr)
					{
						return FReply::Handled();
					}
					if (const UZenoLiveLinkSession* Session = Client->GetSessionFallback(); IsValid(Session))
					{
						zeno::remote::HeightField HeightField = *HeightFieldPtr;
						zeno::remote::SubjectContainer Subject {
							{ TCHAR_TO_ANSI(*Volume->SubjectName) },
							static_cast<uint16>(zeno::remote::ESubjectType::HeightField),
							msgpack::pack(HeightField),
						};
						zeno::remote::SubjectContainerList List {
							{ Subject, },
						};
						Session->GetClient()->SetSubjectToRemote(List);
					}
					return FReply::Handled();
				})
				[
					SNew(STextBlock)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.Text(LOCTEXT("Generate", "Generate"))
				]
			]
		];
}

TSharedRef<IDetailCustomization> FZenoPCGActorDetailCustomization::Create()
{
	return MakeShared<FZenoPCGActorDetailCustomization>();
}

REGISTER_ZENO_DETAIL_CUSTOMIZATION("ZenoPCGVolume", FZenoPCGActorDetailCustomization);

#undef LOCTEXT_NAMESPACE
