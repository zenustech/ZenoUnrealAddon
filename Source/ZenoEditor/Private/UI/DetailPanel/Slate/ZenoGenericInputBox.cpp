// Fill out your copyright notice in the Description page of Project Settings.


#include "ZenoGenericInputBox.h"

#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SZenoGenericInputBox::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SAssignNew(InputBox, SEditableTextBox)
		.OnTextChanged_Raw(this, &SZenoGenericInputBox::OnInputTextChanged)
		.OnTextCommitted_Raw(this, &SZenoGenericInputBox::OnInputTextCommitted)
	];

	
	ParamType = static_cast<zeno::unreal::EParamType>(InArgs._ParamType);
	switch (ParamType)
	{
		case EParamType::Float:
			InputBox->SetText(FText::FromString("0.0"));
		case EParamType::Integer:
			InputBox->SetText(FText::FromString("0"));
		default: ;
	}
}

std::any SZenoGenericInputBox::GetData() const
{
	const FString String = CurrentText.ToString();
	switch (ParamType)
	{
		case EParamType::Float:
			return FCString::Atof(*String);
		case EParamType::Integer:
			return FCString::Atoi(*String);
		default: ;
	}
	return {};
}

void SZenoGenericInputBox::OnInputTextChanged(const FText& TextToChange)
{
	FString String = CurrentText.ToString();
	switch (ParamType)
	{
		case EParamType::Float: ;
		case EParamType::Integer: ;
		default: ;
	}
}

void SZenoGenericInputBox::OnInputTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType)
{
	CurrentText = CommittedText;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
