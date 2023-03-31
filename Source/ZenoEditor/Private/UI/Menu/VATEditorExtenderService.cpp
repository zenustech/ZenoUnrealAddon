#include "UI/Menu/VATEditorExtenderService.h"

#include "AssetToolsModule.h"
#include "LevelEditor.h"
#include "RawMesh.h"
#include "StaticMeshAttributes.h"
#include "ZenoEditorCommand.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Blueprint/ZenoCommonBlueprintLibrary.h"
#include "Blueprint/ZenoEditorSettings.h"
#include "Importer/VAT/VATTypes.h"
#include "Importer/VAT/VATUtility.h"
#include "Importer/Wavefront/ZenoObjLoader.h"
#include "Kismet/KismetStringLibrary.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Misc/FileHelper.h"

#define LOCTEXT_NAMESPACE "FVATEditorExtenderService"

FVATEditorExtenderService::FVATEditorExtenderService()
	: FZenoEditorExtenderServiceBase()
{
}

void FVATEditorExtenderService::Register()
{
	FZenoEditorExtenderServiceBase::Register();
	MenuBarExtender->AddMenuBarExtension("Help", EExtensionHook::After, CommandList, FMenuBarExtensionDelegate::CreateRaw(this, &FVATEditorExtenderService::ExtendMenuBar));
}

void FVATEditorExtenderService::Unregister()
{
	FZenoEditorExtenderServiceBase::Unregister();
}

void FVATEditorExtenderService::ExtendMenuBar(FMenuBarBuilder& Builder)
{
	Builder.AddPullDownMenu(LOCTEXT("Zeno", "Zeno"), LOCTEXT("ZenoTooltip", "Zeno Tools"), FNewMenuDelegate::CreateRaw(this, &FVATEditorExtenderService::ExtendVATPullDownMenu), ZenoHookLabel, ZenoHookLabel);
}

void FVATEditorExtenderService::ExtendVATPullDownMenu(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FZenoEditorCommand::Get().ImportWavefrontMesh);
	Builder.AddMenuEntry(FZenoEditorCommand::Get().Debug);
}

void FVATEditorExtenderService::MapAction()
{
	CommandList->MapAction(FZenoEditorCommand::Get().Debug, FExecuteAction::CreateRaw(this, &FVATEditorExtenderService::Debug));
	CommandList->MapAction(FZenoEditorCommand::Get().ImportWavefrontMesh, FExecuteAction::CreateRaw(this, &FVATEditorExtenderService::ImportWavefrontObjFile));
}

FVATEditorExtenderService& FVATEditorExtenderService::Get()
{
	static FVATEditorExtenderService Service;
	return Service;
}

void FVATEditorExtenderService::Debug()
{
	TArray<FString> Files;
	if (UZenoCommonBlueprintLibrary::OpenSystemFilePicker(Files) && Files.Num() > 0)
	{
		FVATInfo Info;
		UVATUtility::ParseBinaryInfo(Files[0], Info);
	}
}

void FVATEditorExtenderService::ImportWavefrontObjFile()
{
	TArray<FString> Files;
	if (UZenoCommonBlueprintLibrary::OpenSystemFilePicker(Files, "Open File", "", "", "OBJ File(With Zeno VAT Binary)|*.obj") && Files.Num() > 0)
	{
		for (const FString& File : Files)
		{
			ProcessObjFileImport(File);
		}
	}
}

void FVATEditorExtenderService::ProcessObjFileImport(const FString& FilePath)
{
	const FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>(
    			"AssetTools");
	TArray<FString> Arr;
	FFileHelper::LoadFileToStringArray(Arr, *FilePath);
	const FWavefrontFileParser Parser{Arr};
	EWavefrontParseError Err;
	TSharedPtr<FRawMesh> RawMesh = Parser.Parse(Err);
	if (Err == EWavefrontParseError::Success)
	{
		UStaticMesh* StaticMesh = NewObject<UStaticMesh>(GetTransientPackage(), MakeUniqueObjectName(GetTransientPackage(), UStaticMesh::StaticClass()), RF_Public | RF_Standalone);
		StaticMesh->ImportVersion = LastVersion;
		StaticMesh->PreEditChange(nullptr);
		FStaticMeshSourceModel& SourceModel = StaticMesh->AddSourceModel();
		{
			StaticMesh->NaniteSettings.bEnabled = false;
			SourceModel.BuildSettings.bRecomputeNormals = false;
			SourceModel.BuildSettings.bRecomputeTangents = true;
			SourceModel.BuildSettings.bRemoveDegenerates = false;
			SourceModel.BuildSettings.bComputeWeightedNormals = false;
			SourceModel.BuildSettings.bUseMikkTSpace = false;
			SourceModel.BuildSettings.bUseFullPrecisionUVs = true;
			SourceModel.BuildSettings.bUseHighPrecisionTangentBasis = true;
			SourceModel.SaveRawMesh(*RawMesh);
		}
		StaticMesh = Cast<UStaticMesh>(AssetToolsModule.Get().DuplicateAssetWithDialog(MakeUniqueObjectName(GetTransientPackage(), UStaticMesh::StaticClass()).ToString(), "/GAME", StaticMesh));
		if (!IsValid(StaticMesh))
		{
			UE_LOG(LogTemp, Warning, TEXT("User canceled import of %ls"), *FilePath);
			return;
		}
		if (UZenoEditorSettings::Get()->bAutoCreateBasicVatMaterialInstanceConstant)
		{
			UMaterialInstance* NewMI = UZenoEditorSettings::CreateBasicVATMaterialInstance(FString::Printf(TEXT("Mat_PosAndNormVAT_%ls_Inst"), *StaticMesh->GetName()), StaticMesh->GetFullName());
			TryLoadPositionAndNormalVATBinaryDescriptor(FilePath, NewMI);
			StaticMesh->AddMaterial(NewMI);
		}
		else
		{
			FStaticMaterial StaticMaterial;
			StaticMaterial.MaterialSlotName = NAME_None;
			StaticMesh->GetStaticMaterials().Add(StaticMaterial);
		}
		StaticMesh->PostEditChange();
		StaticMesh->Build(false);
		FAssetRegistryModule::AssetCreated(StaticMesh);
		if (IsValid(StaticMesh->GetPackage()))
		{
			auto _ = StaticMesh->GetPackage()->MarkPackageDirty();
		}
	}
}

bool FVATEditorExtenderService::TryLoadPositionAndNormalVATBinaryDescriptor(const FString& InObjPath, UMaterialInstance* MaterialInstance)
{
#if WITH_EDITORONLY_DATA
	const FString TrimString = InObjPath.TrimEnd();
	if (TrimString.EndsWith(".obj"))
	{
		const FString BinaryPath = UKismetStringLibrary::GetSubstring(TrimString, 0, TrimString.Len() - 4);
		if (FPaths::FileExists(BinaryPath))
		{
			if (FVATInfo Info; UVATUtility::ParseBinaryInfo(BinaryPath, Info))
			{
				if (UMaterialInstanceConstant* Constant = Cast<UMaterialInstanceConstant>(MaterialInstance); IsValid(Constant))
				{
					const float FrameNum = static_cast<float>(Info.FrameNum);
					const float Height = static_cast<float>(Info.ImageHeight);
					Constant->SetScalarParameterValueEditorOnly(TEXT("Number of Frames"), FrameNum);
					Constant->SetScalarParameterValueEditorOnly(TEXT("Height"), Height);
					Constant->SetScalarParameterValueEditorOnly(TEXT("BBMaxX"), Info.BoundingBoxMax.at(0));
					Constant->SetScalarParameterValueEditorOnly(TEXT("BBMaxY"), Info.BoundingBoxMax.at(2));
					Constant->SetScalarParameterValueEditorOnly(TEXT("BBMaxZ"), Info.BoundingBoxMax.at(1));
					Constant->SetScalarParameterValueEditorOnly(TEXT("BBMinX"), Info.BoundingBoxMin.at(0));
					Constant->SetScalarParameterValueEditorOnly(TEXT("BBMinY"), Info.BoundingBoxMin.at(2));
					Constant->SetScalarParameterValueEditorOnly(TEXT("BBMinZ"), Info.BoundingBoxMin.at(1));
					return true;
				}
			}
		}
	}
#endif // WITH_EDITORONLY_DATA
	return false;
}

#undef LOCTEXT_NAMESPACE

REGISTER_EDITOR_EXTENDER_SERVICE("VAT", FVATEditorExtenderService);
