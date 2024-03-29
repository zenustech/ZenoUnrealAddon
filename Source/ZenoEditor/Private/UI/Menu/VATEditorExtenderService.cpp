﻿#include "UI/Menu/VATEditorExtenderService.h"

#include "AssetToolsModule.h"
#include "MaterialDomain.h"
#include "RawMesh.h"
#include "StaticMeshOperations.h"
#include "ZenoEditorCommand.h"
#include "ZenoMeshDescriptor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Blueprint/ZenoCommonBlueprintLibrary.h"
#include "Blueprint/ZenoEditorSettings.h"
#include "Blueprint/Panel/VATImportSettings.h"
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
	MenuBarExtender->AddMenuBarExtension("Help", EExtensionHook::Before, CommandList, FMenuBarExtensionDelegate::CreateRaw(this, &FVATEditorExtenderService::ExtendMenuBar));
}

void FVATEditorExtenderService::Unregister()
{
	FZenoEditorExtenderServiceBase::Unregister();
}

void FVATEditorExtenderService::ExtendMenuBar(FMenuBarBuilder& Builder)
{
	Builder.AddPullDownMenu(LOCTEXT("Zeno", "Zeno"), LOCTEXT("ZenoTooltip", "Zeno Tools"), FNewMenuDelegate::CreateRaw(this, &FVATEditorExtenderService::ExtendVATPullDownMenu), ZenoVATHookLabel, ZenoVATHookLabel);
}

void FVATEditorExtenderService::ExtendVATPullDownMenu(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FZenoEditorCommand::Get().ImportVAT);
}

void FVATEditorExtenderService::MapAction()
{
	CommandList->MapAction(FZenoEditorCommand::Get().ImportVAT, FExecuteAction::CreateRaw(this, &FVATEditorExtenderService::ImportVAT));
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

void FVATEditorExtenderService::ImportVAT()
{
	UVATImportSettings* ImportSettings = NewObject<UVATImportSettings>(GetTransientPackage(), UVATImportSettings::StaticClass(), NAME_None, RF_Transient);
	FWavefrontObjectContextCreateArgs ContextCreateArgs;
	const TSharedRef<FStructOnScope> StructOnScope = MakeShared<FStructOnScope>(FWavefrontObjectContextCreateArgs::StaticStruct(), reinterpret_cast<uint8*>(&ContextCreateArgs));
	bool bContinue = UZenoCommonBlueprintLibrary::OpenSettingsModal(ImportSettings, LOCTEXT("ImportSettings", "Import Settings"));
	bContinue = bContinue && UZenoCommonBlueprintLibrary::OpenSettingsModal(StructOnScope, LOCTEXT("ParseSettings", "Parse Settings"));

	const FString& FilePath = ImportSettings->FilePath.FilePath;

	if (!FilePath.EndsWith(".obj"))
	{
		UZenoCommonBlueprintLibrary::ShowNotification(LOCTEXT("FileTypeNotObj", "File type must be .obj"));
		return;
	}
	
	if (!bContinue || FilePath.IsEmpty() || !FPaths::FileExists(FilePath))	
	{
		UZenoCommonBlueprintLibrary::ShowNotification(LOCTEXT("FileNotFound", "File not found"));
		return;
	}

	FWavefrontObjectContext Context(ContextCreateArgs);
	
	TArray<FString> Arr;
	FFileHelper::LoadFileToStringArray(Arr, *FilePath);
	const FWavefrontFileParser Parser{Arr};
	Parser.ParseFile(FZenoWavefrontObjectParserDelegate::CreateLambda([&] (const EWavefrontAttrType& InAttrType, const FString& InData) mutable
	{
		Context.Parse(InAttrType, InData);
	}));
	Context.CompleteParse();
	// const TSharedRef<FRawMesh> RawMesh = Context.ToRawMesh();
	// SaveRawMeshToStaticMesh(*RawMesh);
	// SaveContextToStaticMesh(Context);
	if (const FString ContentPath = UZenoCommonBlueprintLibrary::OpenContentPicker(); !ContentPath.IsEmpty())
	{
		const FString SavePackageName = FPackageName::ObjectPathToPackageName(ContentPath);
		const FString SavePackagePath = FPaths::GetPath(SavePackageName);
		const FString SaveAssetName = FPaths::GetBaseFilename(SavePackageName);

		UPackage* Package = CreatePackage(*SavePackageName);
		
		UZenoMeshInstance* Instance = Context.CreateMeshInstance(Package, *SaveAssetName, ImportSettings->Type == EZenoVATType::DynamicRemesh);
		Instance->VatMeshData.RowsPerFrame = Context.Metadata.RowsPerFrame;
		Instance->VatMeshData.BoundsMax = Context.Metadata.BoundsMax;
		Instance->VatMeshData.BoundsMin = Context.Metadata.BoundsMin;
		Instance->VatMeshData.TextureWidth = Context.Metadata.TextureWidth;
		Instance->VatMeshData.FrameNum = Context.Metadata.FrameNum;
		FAssetRegistryModule::AssetCreated(Instance);
	}
}

void FVATEditorExtenderService::ProcessObjFileImport(const FString& FilePath)
{
	TArray<FString> Arr;
	FFileHelper::LoadFileToStringArray(Arr, *FilePath);
	const FWavefrontFileParser Parser{Arr};
	EWavefrontParseError Err;
	const TSharedPtr<FRawMesh> RawMesh = Parser.ParseDynamicMesh(Err);
	if (Err == EWavefrontParseError::Success)
	{
		SaveRawMeshToStaticMesh(*RawMesh);
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

bool FVATEditorExtenderService::SaveRawMeshToStaticMesh(FRawMesh& InRawMesh)
{
	if (!InRawMesh.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Trying to save an invalid Mesh! aborted."));
		return false;
	}
	
	const FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>(
    			"AssetTools");
	
	if (const FString ContentPath = UZenoCommonBlueprintLibrary::OpenContentPicker(); !ContentPath.IsEmpty())
	{
		const FString SavePackageName = FPackageName::ObjectPathToPackageName(ContentPath);
		const FString SavePackagePath = FPaths::GetPath(SavePackageName);
		const FString SaveAssetName = FPaths::GetBaseFilename(SavePackageName);

		UPackage* Package = CreatePackage(*SavePackageName);
		UStaticMesh* StaticMesh = NewObject<UStaticMesh>(Package, MakeUniqueObjectName(Package, UStaticMesh::StaticClass()), RF_Public | RF_Standalone);
		StaticMesh->ImportVersion = LastVersion;
		StaticMesh->PreEditChange(nullptr);
		FStaticMeshSourceModel& SourceModel = StaticMesh->AddSourceModel();
		{
			StaticMesh->NaniteSettings.bEnabled = false;
			SourceModel.BuildSettings.bRecomputeNormals = false;
			SourceModel.BuildSettings.bRecomputeTangents = false;
			SourceModel.BuildSettings.bRemoveDegenerates = false;
			SourceModel.BuildSettings.bComputeWeightedNormals = true;
			SourceModel.BuildSettings.bUseMikkTSpace = false;
			SourceModel.BuildSettings.bUseFullPrecisionUVs = true;
			SourceModel.BuildSettings.bUseHighPrecisionTangentBasis = true;
			SourceModel.BuildSettings.bGenerateLightmapUVs = false;
			SourceModel.SaveRawMesh(InRawMesh);
		}
		StaticMesh->SetLightingGuid();
		StaticMesh->SetLightMapResolution(64);
		StaticMesh->SetLightMapCoordinateIndex(2);
		StaticMesh->PostEditChange();
		StaticMesh->Build(false);
		
		if (!IsValid(StaticMesh))
		{
			UE_LOG(LogTemp, Warning, TEXT("User canceled import."));
			return false;
		}
		if (UZenoEditorSettings::Get()->bAutoCreateBasicVatMaterialInstanceConstant)
		{
			UMaterialInstance* NewMI = UZenoEditorSettings::CreateBasicVATMaterialInstance(FString::Printf(TEXT("Mat_PosAndNormVAT_%ls_Inst"), *StaticMesh->GetName()), StaticMesh->GetFullName());
			StaticMesh->AddMaterial(NewMI);
		}
		else
		{
			StaticMesh->GetStaticMaterials().Add({});
		}
		FAssetRegistryModule::AssetCreated(StaticMesh);
		if (IsValid(StaticMesh->GetPackage()))
		{
			auto _ = StaticMesh->GetPackage()->MarkPackageDirty();
		}
		return true;
	}

	return false;
}

bool FVATEditorExtenderService::SaveContextToStaticMesh(const FWavefrontObjectContext& InContext)
{
	const FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>(
    			"AssetTools");
	
	if (const FString ContentPath = UZenoCommonBlueprintLibrary::OpenContentPicker(); !ContentPath.IsEmpty())
	{
		const FString SavePackageName = FPackageName::ObjectPathToPackageName(ContentPath);
		const FString SavePackagePath = FPaths::GetPath(SavePackageName);
		const FString SaveAssetName = FPaths::GetBaseFilename(SavePackageName);

		UPackage* Package = CreatePackage(*SavePackageName);
		UStaticMesh* StaticMesh = NewObject<UStaticMesh>(Package, MakeUniqueObjectName(Package, UStaticMesh::StaticClass()), RF_Public | RF_Standalone);
		StaticMesh->ImportVersion = LastVersion;
		StaticMesh->PreEditChange(nullptr);
		FStaticMeshSourceModel& SourceModel = StaticMesh->AddSourceModel();
		{
			StaticMesh->NaniteSettings.bEnabled = false;
			SourceModel.BuildSettings.bRecomputeNormals = false;
			SourceModel.BuildSettings.bRecomputeTangents = false;
			SourceModel.BuildSettings.bRemoveDegenerates = false;
			SourceModel.BuildSettings.bComputeWeightedNormals = true;
			SourceModel.BuildSettings.bUseMikkTSpace = false;
			SourceModel.BuildSettings.bUseFullPrecisionUVs = true;
			SourceModel.BuildSettings.bUseHighPrecisionTangentBasis = true;
			SourceModel.BuildSettings.bGenerateLightmapUVs = true;
			SourceModel.BuildSettings.SrcLightmapIndex = 0;
			SourceModel.BuildSettings.DstLightmapIndex = 2;
			FMeshDescription* MeshDescription = SourceModel.CreateMeshDescription();
			InContext.FillMeshDescription(MeshDescription);
			SourceModel.CommitMeshDescription(true);
		}
		StaticMesh->PostEditChange();
		StaticMesh->Build(false);
		
		if (!IsValid(StaticMesh))
		{
			UE_LOG(LogTemp, Warning, TEXT("User canceled import."));
			return false;
		}
		if (UZenoEditorSettings::Get()->bAutoCreateBasicVatMaterialInstanceConstant)
		{
			UMaterialInstance* NewMI = UZenoEditorSettings::CreateBasicVATMaterialInstance(FString::Printf(TEXT("Mat_PosAndNormVAT_%ls_Inst"), *StaticMesh->GetName()), StaticMesh->GetFullName());
			StaticMesh->AddMaterial(NewMI);
		}
		else
		{
			StaticMesh->GetStaticMaterials().Add({});
		}
		FAssetRegistryModule::AssetCreated(StaticMesh);
		if (IsValid(StaticMesh->GetPackage()))
		{
			auto _ = StaticMesh->GetPackage()->MarkPackageDirty();
		}
		return true;
	}

	return false;
}

#undef LOCTEXT_NAMESPACE

REGISTER_EDITOR_EXTENDER_SERVICE("VAT", FVATEditorExtenderService);
