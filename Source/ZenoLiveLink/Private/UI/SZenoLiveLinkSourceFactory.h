// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IStructureDetailsView.h"
#include "ZenoLiveLinkSetting.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE_OneParam(FOnZenoLiveLinkConnectionSettingsAccepted, FZenoLiveLinkSetting);

class ZENOLIVELINK_API SZenoLiveLinkSourceFactory : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SZenoLiveLinkSourceFactory)
		{}
		SLATE_EVENT(FOnZenoLiveLinkConnectionSettingsAccepted, OnConnectionSettingsAccepted)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:
	FZenoLiveLinkSetting ConnectionSettings;

#if WITH_EDITOR
	TSharedPtr<FStructOnScope> StructOnScope;
	TSharedPtr<IStructureDetailsView> StructureDetailsView;
#endif // WITH_EDITOR
	
	FReply OnSettingsAccepted();
	FOnZenoLiveLinkConnectionSettingsAccepted OnConnectionSettingsAccepted;
};
