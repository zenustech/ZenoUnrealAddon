#include "ZenoGraphMeshActorDetailCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "SlateOptMacros.h"
#include "ZenoGraphActor.h"
#include "ZenoGraphAsset.h"
#include "Async/Async.h"
#include "Client/ZenoLiveLinkClientSubsystem.h"
#include "Input/ZenoInputParameter.h"
#include "UI/DetailPanel/ZenoDetailPanelService.h"
#include "UObject/GCObjectScopeGuard.h"

#define LOCTEXT_NAMESPACE "FZenoGraphMeshActorDetailCustomization"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void FZenoGraphMeshActorDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	AZenoGraphMeshActor* MeshActor;
	{
		TArray<TWeakObjectPtr<UObject>> Objects;
		DetailBuilder.GetObjectsBeingCustomized(Objects);
		check(!Objects.IsEmpty());
		MeshActor = Cast<AZenoGraphMeshActor>(Objects[0]);
		if (!IsValid(MeshActor) || !IsValid(MeshActor->ZenoGraphAsset)) return;
	}

	const FName ZenoCategoryName("Zeno");
	IDetailCategoryBuilder& ZenoBuilder = DetailBuilder.EditCategory(ZenoCategoryName, FText::GetEmpty(), ECategoryPriority::Important);
	FDetailWidgetRow& NewRow = ZenoBuilder.AddCustomRow(FText::GetEmpty());
	NewRow.ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		  .AutoWidth()
		  .Padding(4.0f)
		[
			SNew(SButton)
			.OnClicked_Lambda([MeshActor] ()
			{
				if (!IsValid(MeshActor)) return FReply::Handled();
				FGCObjectScopeGuard ScopeGuard { MeshActor };
				UZenoLiveLinkClientSubsystem* Client = GEngine->GetEngineSubsystem<UZenoLiveLinkClientSubsystem>();
				if (UZenoLiveLinkSession* Session = Client->GetSessionFallback(); IsValid(Session))
				{
					zeno::remote::GraphRunInfo RunInfo;
					RunInfo.GraphDefinition = TCHAR_TO_ANSI(*MeshActor->ZenoGraphAsset->ZenoActionRecordExportedData);
					for (const UZenoInputParameter* Input : MeshActor->InputParameters)
					{
						RunInfo.Values.Values.push_back(Input->GatherParamValue());
					}
					AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [Session, RunInfo] () mutable
					{
						  Session->GetClient()->RunGraph(RunInfo)->GetFuture().Then([] (const TResultFuture<bool>& Result)
						  {
							  UE_LOG(LogTemp, Error, TEXT("123: %d %d"), Result.Get().IsSet(), Result.Get().Get(false));
						  });
					});
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

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TSharedRef<IDetailCustomization> FZenoGraphMeshActorDetailCustomization::Create()
{
	return MakeShared<FZenoGraphMeshActorDetailCustomization>();
}

#undef LOCTEXT_NAMESPACE

REGISTER_ZENO_DETAIL_CUSTOMIZATION("ZenoGraphMeshActor", FZenoGraphMeshActorDetailCustomization);
