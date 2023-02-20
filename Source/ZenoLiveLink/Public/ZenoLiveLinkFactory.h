// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#define LOCTEXT_NAMESPACE "ZenoLiveLinkFactory"

#include "CoreMinimal.h"
#include "LiveLinkSourceFactory.h"
#include "ZenoLiveLinkSetting.h"
#include "ZenoLiveLinkFactory.generated.h"

/**
 * 
 */
UCLASS()
class ZENOLIVELINK_API UZenoLiveLinkFactory : public ULiveLinkSourceFactory
{
	GENERATED_BODY()

public:
	// ~start overriding ULiveLinkSourceFactory
	virtual FText GetSourceDisplayName() const override;
	virtual FText GetSourceTooltip() const override;
	virtual TSharedPtr<SWidget> CreateSourceCreationPanel() override;
	virtual TSharedPtr<ILiveLinkSource> OnSourceCreationPanelClosed(bool bMakeSource) override;

	virtual TSharedPtr<ILiveLinkSource> CreateSource(const FString& ConnectionString) const override;

	virtual EMenuType GetMenuType() const override { return EMenuType::SubPanel; }
	virtual TSharedPtr<SWidget> BuildCreationPanel(FOnLiveLinkSourceCreated OnLiveLinkSourceCreated) const override;
	// ~end overriding ULiveLinkSourceFactory

private:
	/**
	 * Notify UE that a source created without using CreateSource interface.
	 * Binding to "Accept" button now.
	 */
	virtual void CreateSourceFromSettings(FZenoLiveLinkSetting InSettings, FOnLiveLinkSourceCreated OnSourceCreated) const;
};
