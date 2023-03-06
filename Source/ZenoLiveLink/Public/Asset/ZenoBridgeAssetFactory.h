// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"
#include "UObject/Object.h"
#include "ZenoBridgeAssetFactory.generated.h"

class UZenoBridgeAsset;

/**
 * 
 */
UCLASS(HideCategories=Object, MinimalAPI)
class UZenoBridgeAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UZenoBridgeAssetFactory();

	virtual bool ShouldShowInNewMenu() const override;

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
};

class ZENOLIVELINK_API FZenoBridgeAssetActions : public FAssetTypeActions_Base
{

public:
	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual FColor GetTypeColor() const override;
	virtual uint32 GetCategories() override;
	
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor) override;
	
};

class ZENOLIVELINK_API FZenoBridgeAssetToolkits : public FAssetEditorToolkit
{
public:
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;

	void InitZenoBridgeEditor(const EToolkitMode::Type InToolkitMode, const TSharedPtr<class IToolkitHost>& InitToolkitHost, UZenoBridgeAsset* InAsset);

private:
	TWeakObjectPtr<UZenoBridgeAsset> WeakAsset;
	TSharedPtr<class FDocumentTracker> DocumentTracker;
};
