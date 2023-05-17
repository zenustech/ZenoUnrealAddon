// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LiveLinkInterface/Public/LiveLinkSourceFactory.h"
#include "ZenoLiveLinkFactory.generated.h"

struct FZenoLiveLinkSetting;

UCLASS()
class ZENOLIVELINKRUNTIME_API UZenoLiveLinkFactory : public ULiveLinkSourceFactory
{
	GENERATED_BODY()

public:
	// ~Impl ULiveLinkSourceFactory start
	virtual FText GetSourceDisplayName() const override;
	virtual FText GetSourceTooltip() const override;
	
#ifdef WITH_EDITORONLY_DATA
	virtual TSharedPtr<SWidget> BuildCreationPanel(FOnLiveLinkSourceCreated OnLiveLinkSourceCreated) const override;
#endif // WITH_EDITORONLY_DATA

	virtual TSharedPtr<ILiveLinkSource> CreateSource(const FString& ConnectionString) const override;

	virtual EMenuType GetMenuType() const override;
	// ~Impl ULiveLinkSourceFactory end

protected:
	// ~Impl ULiveLinkSourceFactory start
	/**
	* Notify UE that a source created without using CreateSource interface.
	* Binding to "Accept" button now.
	*/
	virtual void CreateSourceFromSettings(FZenoLiveLinkSetting InSettings, FOnLiveLinkSourceCreated OnSourceCreated) const;
	// ~Impl ULiveLinkSourceFactory end
};
