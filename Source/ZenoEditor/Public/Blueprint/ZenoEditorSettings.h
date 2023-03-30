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

public:
	UZenoEditorSettings();
	/* Wrapper to get CDO of UZenoEditorSettings */
	UFUNCTION(BlueprintCallable)
	static const UZenoEditorSettings* Get();

	UFUNCTION(BlueprintCallable)
	static UMaterialInstance* CreateBasicVATMaterialInstance(const FString& InName, const FString& PackagePath);
};
