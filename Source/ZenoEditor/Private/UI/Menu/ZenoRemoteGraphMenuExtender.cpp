#include "UI/Menu/ZenoRemoteGraphMenuExtender.h"

#include "ZenoEditorCommand.h"
#include "ZenoGraphAsset.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Async/Async.h"
#include "Blueprint/ZenoCommonBlueprintLibrary.h"
#include "Client/ZenoLiveLinkClientSubsystem.h"
#include "Misc/FileHelper.h"
#include "UObject/GCObjectScopeGuard.h"
#include "Utilities/ZenoEngineTypes.h"

#define LOCTEXT_NAMESPACE "FZenoRemoteGraphMenuExtender"

void FZenoRemoteGraphMenuExtender::Register()
{
	FZenoEditorExtenderServiceBase::Register();
	MenuBarExtender->AddMenuBarExtension("Help", EExtensionHook::After, CommandList, FMenuBarExtensionDelegate::CreateRaw(this, &FZenoRemoteGraphMenuExtender::ExtendTopMenuBar));
}

void FZenoRemoteGraphMenuExtender::Unregister()
{
	FZenoEditorExtenderServiceBase::Unregister();
}

void FZenoRemoteGraphMenuExtender::MapAction()
{
	CommandList->MapAction(FZenoEditorCommand::Get().Debug, FExecuteAction::CreateRaw(this, &FZenoRemoteGraphMenuExtender::Debug));
	CommandList->MapAction(FZenoEditorCommand::Get().ImportZslFile, FExecuteAction::CreateRaw(this, &FZenoRemoteGraphMenuExtender::ImportZslFile));
}

void FZenoRemoteGraphMenuExtender::ExtendTopMenuBar(FMenuBarBuilder& Builder)
{
	Builder.AddPullDownMenu(LOCTEXT("ZenoGraph", "Zeno Graph"), LOCTEXT("ZenoGraphTooltips", "Graph actions"), FNewMenuDelegate::CreateRaw(this, &FZenoRemoteGraphMenuExtender::ExtendEmbedPullDownMenu));
}

void FZenoRemoteGraphMenuExtender::ExtendEmbedPullDownMenu(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FZenoEditorCommand::Get().ZenoGraphHeader);
	Builder.AddSeparator();
	Builder.AddMenuEntry(FZenoEditorCommand::Get().ImportZslFile);
	Builder.AddMenuEntry(FZenoEditorCommand::Get().Debug);
}

void FZenoRemoteGraphMenuExtender::Debug()
{
}

void FZenoRemoteGraphMenuExtender::ImportZslFile()
{
	UZenoLiveLinkClientSubsystem* LiveLinkSubsystem = GEngine->GetEngineSubsystem<UZenoLiveLinkClientSubsystem>();
	UZenoLiveLinkSession* Session = LiveLinkSubsystem->GetSessionFallback();
	if (!IsValid(Session)) {return;}

	// Let user pick some files, blocking
	TArray<FString> Files;
	if (UZenoCommonBlueprintLibrary::OpenSystemFilePicker(Files, "Open File", "", "", "ZSL File|*.json") && Files.Num() > 0)
	{
		for (const FString& File : Files)
		{
			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [Session, File]()
			{
				if (!IsValid(Session)) { return; }
				FGCObjectScopeGuard ScopeGuard { Session };
				
				FString JsonData;
				// Try to load file into JsonData
				if (!FFileHelper::LoadFileToString(JsonData, *File)) { return; }
				
				TSharedPromise<zeno::remote::GraphInfo> Promise = Session->GetClient()->TryParseGraphInfo(JsonData);
				Promise->GetFuture().Then([File, JsonData] (const TResultFuture<zeno::remote::GraphInfo>& Result)
				{
					const TOptional<zeno::remote::GraphInfo> OptionalObj = Result.Get();
					if (OptionalObj.IsSet())
					{
						zeno::remote::GraphInfo GraphInfo = OptionalObj.GetValue();
						// Save graph info to asset
						if (const FString ContentPath = UZenoCommonBlueprintLibrary::OpenContentPicker(); !ContentPath.IsEmpty())
						{
							const FString SavePackageName = FPackageName::ObjectPathToPackageName(ContentPath);
							const FString SavePackagePath = FPaths::GetPath(SavePackageName);
							const FString SaveAssetName = FPaths::GetBaseFilename(SavePackageName);

							UPackage* Package = CreatePackage(*SavePackageName);
							UZenoGraphAsset* Asset = NewObject<UZenoGraphAsset>(Package, UZenoGraphAsset::StaticClass(), FName(SaveAssetName), RF_Public | RF_Standalone);
							if (IsValid(Asset))
							{
								Asset->PreEditChange(nullptr);
								Asset->ReimportPath = File;
								Asset->ZenoActionRecordExportedData = JsonData;
								for (const auto& Pair : GraphInfo.InputParameters)
								{
									Asset->InputParameterDescriptors.Add(FZenoInputParameterDescriptor::FromZenoType(Pair.second));
								}
								for (const auto& Pair : GraphInfo.OutputParameters)
								{
									Asset->OutputParameterDescriptors.Add(FZenoOutputParameterDescriptor::FromZenoType(Pair.second));
								}
								Asset->PostEditChange();
								FAssetRegistryModule::AssetCreated(Asset);
							}
						}
					}
				});
			});
		}
	}
}

#undef LOCTEXT_NAMESPACE

REGISTER_EDITOR_EXTENDER_SERVICE("RemoteGraph", FZenoRemoteGraphMenuExtender);
