// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <any>
#include <zeno/unreal/ZenoUnrealTypes.h>

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class ZENOEDITOR_API SZenoGenericInputBox : public SBox
{
	
public:
	using EParamType = zeno::unreal::EParamType;
	
	SLATE_BEGIN_ARGS(SZenoGenericInputBox)
		: _ParamType(static_cast<int8>(zeno::unreal::EParamType::Invalid))
	{}
		SLATE_ARGUMENT(int8, ParamType)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	std::any GetData() const;

	template<typename T>
	T GetDataChecked(T DefaultValue = {}) const
	{
		try
		{
			return std::any_cast<T>(GetData());
		}
		catch (...)
		{
			return DefaultValue;
		}
	}

	template<typename T>
	T GetDataUnsafe() const
	{
		return std::any_cast<T>(GetData());
	}
	

protected:
	virtual void OnInputTextChanged(const FText& TextToChange);
	virtual void OnInputTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType);

private:
	TSharedPtr<SEditableTextBox> InputBox;
	FText CurrentText;
	EParamType ParamType = EParamType::Invalid;
};
