#include "UI/DetailPanel/ZenoPCGActorDetailCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "LandscapeComponent.h"
#include "RawMesh.h"
#include "ZenoGraphAsset.h"
#include "Client/ZenoLiveLinkClientSubsystem.h"
#include "Input/ZenoInputParameter.h"
#include "PCG/ZenoPCGVolume.h"
#include "PCG/ZenoPCGVolumeComponent.h"
#include "UI/DetailPanel/ZenoDetailPanelService.h"
#include "UObject/GCObjectScopeGuard.h"

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
						// Push height field to zeno remote
						zeno::remote::HeightField HeightField = *HeightFieldPtr;
						zeno::remote::SubjectContainer Subject {
							{ TCHAR_TO_ANSI(*Volume->SubjectName) },
							static_cast<uint16>(zeno::remote::ESubjectType::HeightField),
							msgpack::pack(HeightField),
						};
						zeno::remote::SubjectContainerList List {
							{ Subject, },
						};
						Session->GetClient()->SetSubjectToRemote(List)->GetFuture()
						.Then([Session, Volume] (const TResultFuture<bool>& Future)
						{
							if (Future.Get().Get(false) && IsValid(Session) && IsValid(Volume) && IsValid(Volume->PCGComponent))
							{
								FGCObjectScopeGuard SessionGuard { Session };
								FGCObjectScopeGuard VolumeGuard { Volume };
								zeno::remote::GraphRunInfo RunInfo;
								const UZenoGraphAsset* GraphAsset = Volume->PCGComponent->ZenoGraph.LoadSynchronous();
								RunInfo.GraphDefinition = TCHAR_TO_ANSI(*GraphAsset->ZenoActionRecordExportedData);
								for (const UZenoInputParameter* Input : Volume->PCGComponent->InputParameters)
								{
									 RunInfo.Values.Values.push_back(Input->GatherParamValue());
								}
								Session->GetClient()->RunGraph(RunInfo)->GetFuture()
								.Then([Session, Volume] (const TResultFuture<bool>& RunFuture)
								{
									if (RunFuture.Get().Get(false) && IsValid(Session) && IsValid(Volume))
									{
										FGCObjectScopeGuard SessionGuard1 { Session };
										FGCObjectScopeGuard VolumeGuard1 { Volume };
										// Load result mesh from zeno remote
										const UZenoGraphAsset* GraphInfo = Volume->PCGComponent->ZenoGraph.LoadSynchronous();
										if (!GraphInfo->OutputParameterDescriptors.IsEmpty())
										{
											const TOptional<const FZenoOutputParameterDescriptor> Output = GraphInfo->FindFirstOutputParameterDescriptor(EZenoSubjectType::Mesh);
											if (Output.IsSet())
											{
												const FString ResultName = Output.GetValue().Name;
												UZenoLiveLinkClientSubsystem* LiveLinkSubsystem = GEngine->GetEngineSubsystem<UZenoLiveLinkClientSubsystem>();
												LiveLinkSubsystem->TryLoadSubjectRemotely<zeno::remote::Mesh>(FName(ResultName))->GetFuture()
												.Then([Volume] (const TResultFuture<zeno::remote::Mesh>& MeshFuture)
												{
													if (MeshFuture.Get().IsSet() && IsValid(Volume))
													{
														FGCObjectScopeGuard VolumeGuard2 { Volume };
														const zeno::remote::Mesh MeshData = MeshFuture.Get().GetValue();
														FRawMesh RawMesh = UZenoLiveLinkClientSubsystem::ConvertZenoMeshToRawMesh(MeshData);
														Volume->OnGeneratedNewMesh(RawMesh);
													}
												});
											}
										}
									}
								});
							}
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

TSharedRef<IDetailCustomization> FZenoPCGActorDetailCustomization::Create()
{
	return MakeShared<FZenoPCGActorDetailCustomization>();
}

REGISTER_ZENO_DETAIL_CUSTOMIZATION("ZenoPCGVolume", FZenoPCGActorDetailCustomization);

#undef LOCTEXT_NAMESPACE
