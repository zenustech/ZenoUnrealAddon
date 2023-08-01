// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZenoEditorSettings.generated.h"

UCLASS(Config=Plugins, DefaultConfig, meta = (DisplayName = "Zeno Editor"))
class ZENOEDITOR_API UZenoEditorSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	/* Enable MIC auto creating */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "VAT")
	bool bAutoCreateBasicVatMaterialInstanceConstant;
	
	/* Parent material to create MaterialInstance */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "VAT", AdvancedDisplay)
	TSoftObjectPtr<UMaterialInterface> DefaultBasicVATMaterial;

	/* Wavefront obj file import scale, apply while reading obj file. Set to smaller value to get high precious. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "VAT")
	FVector3f VATImportScale;

	/* Default Imported Landscape Material Path */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Asset")
	TSoftObjectPtr<UMaterialInterface> DefaultImportedLandscapeMaterialParent;

public:
	UZenoEditorSettings();
	/* Wrapper to get CDO of UZenoEditorSettings */
	UFUNCTION(BlueprintCallable)
	static const UZenoEditorSettings* Get();

	UFUNCTION(BlueprintCallable)
	static UMaterialInstance* CreateBasicVATMaterialInstance(const FString& InName, const FString& PackagePath);
};
