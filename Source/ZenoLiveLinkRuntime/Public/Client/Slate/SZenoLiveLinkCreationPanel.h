// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class IStructureDetailsView;
struct FZenoLiveLinkSetting;

DECLARE_DELEGATE_OneParam(FOnZenoLiveLinkConnectionSettingsAccepted, FZenoLiveLinkSetting);

/**
 * Slate UI for LiveLink window
 */
class ZENOLIVELINKRUNTIME_API SZenoLiveLinkCreationPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SZenoLiveLinkCreationPanel)
		{}
		SLATE_EVENT(FOnZenoLiveLinkConnectionSettingsAccepted, OnConnectionSettingsAccepted)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

protected:
	FReply OnSettingAccepted() const;
	FOnZenoLiveLinkConnectionSettingsAccepted OnConnectionSettingsAccepted;

private:
#if WITH_EDITOR
	TSharedPtr<FStructOnScope> StructOnScope;
	TSharedPtr<IStructureDetailsView> StructureDetailsView;
#endif // WITH_EDITOR
};
