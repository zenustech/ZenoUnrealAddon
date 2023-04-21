// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SComboBox.h"

class ZENOEDITOR_API SEnumComboBox : public SBox
{
public:
	SLATE_BEGIN_ARGS(SEnumComboBox)
		: _Font()
		, _EnumClassPath("/Script/ZenoEngine.EZenoParamType")
	{}
		/** Font color and opacity (overrides Style) */
		SLATE_ATTRIBUTE( FSlateFontInfo, Font )
		/** FName of target enum class */
		SLATE_ARGUMENT( FString, EnumClassPath )
	SLATE_END_ARGS()

	typedef TSharedPtr<FString> FComboItemType;

	SEnumComboBox();

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	TSharedRef<SWidget> MakeWidgetForOption(FComboItemType InOption) const;

	void OnSelectionChanged(FComboItemType NewValue, ESelectInfo::Type);

	FText GetCurrentLabel() const;

protected:
	TAttribute<FSlateFontInfo> FontInfo;
	FComboItemType CurrentItem;
	TArray<FComboItemType> Options;
};

