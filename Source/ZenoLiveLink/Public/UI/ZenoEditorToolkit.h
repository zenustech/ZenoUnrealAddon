// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ZenoEditorToolkit.generated.h"

DECLARE_DELEGATE_TwoParams(FZenoEditorToolkitBuildToolPalette, class FToolBarBuilder&, const TSharedRef<SVerticalBox>);

UCLASS()
class ZENOLIVELINK_API UZenoEditorToolkit : public UObject
{
	GENERATED_BODY()

public:
	explicit UZenoEditorToolkit(const FObjectInitializer& ObjectInitializer);

	/**
	 * Init command list, must be override
	 */
	virtual void Init();
	
	virtual void Register();
	virtual void UnRegister();

	virtual bool CanBeCreate(const FSpawnTabArgs& Args);

	void InvokeUI();
	void RequestToClose();

protected:
	FText DisplayName;
	ETabSpawnerMenuType::Type MenuType;
	FName TabId;

	TSharedRef<FUICommandList> UICommandList;
	TMap<FName, FZenoEditorToolkitBuildToolPalette> ToolPaletteBuilders;
	TMap<FName, TSharedRef<FUICommandInfo>> ModeInfos;
	FName CurrentMode;
	
	void AddMode(FName InNewMode, const TSharedRef<FUICommandInfo> CommandInfo, FZenoEditorToolkitBuildToolPalette Builder);
	
	virtual void OnChangeMode(FName ModeName);
	virtual bool IsModeEnabled(FName ModeName) const;
	virtual bool IsModeActive(FName ModeName) const;
	
	virtual void MakeDockTab(FName InMode);

private:
	TSharedRef<SDockTab> GetDockTab(const FSpawnTabArgs& Args);

	TSharedPtr<SDockTab> Slate_ToolkitDockTab;
};
