#include "UI/Common/EnumComboBox.h"

#define LOCTEXT_NAMESPACE "SEnumComboBox"

SEnumComboBox::SEnumComboBox()
{}

void SEnumComboBox::Construct(const FArguments& InArgs)
{
	FontInfo = InArgs._Font;

	const UEnum* Enum = FindObject<UEnum>(nullptr, *InArgs._EnumClassPath, false);
	int32 DefaultEnumIdx = 0;
	if (Enum != nullptr)
	{
		for (int32 EnumIdx = 0; EnumIdx < Enum->NumEnums() - 1; ++EnumIdx)
		{
			FString Excerpt = Enum->GetNameStringByIndex(EnumIdx);

			const bool bShouldBeHidden = Enum->HasMetaData(TEXT("Hidden"), EnumIdx) || Enum->HasMetaData(TEXT("Spacer"), EnumIdx);
			if (!bShouldBeHidden)
			{
				Options.Add(MakeShared<FString>(MoveTempIfPossible(Excerpt)));
			}

			const bool bIsDefaultEnumValue = Enum->HasMetaData(TEXT("Default"), EnumIdx);
			if (bIsDefaultEnumValue)
			{
				DefaultEnumIdx = EnumIdx;
			}
		}
	}

	if (!CurrentItem.IsValid() && Options.Num() > 0)
	{
		CurrentItem = Options[DefaultEnumIdx];
	}
		
	ChildSlot
	[
		SNew(SComboBox<FComboItemType>)
			.OptionsSource(&Options)
			.OnGenerateWidget_Raw(this, &SEnumComboBox::MakeWidgetForOption)
			.OnSelectionChanged_Raw(this, &SEnumComboBox::OnSelectionChanged)
			.InitiallySelectedItem(CurrentItem)
		[
			SNew(STextBlock)
				.Text_Raw(this, &SEnumComboBox::GetCurrentLabel)
				.Font(FontInfo)
		]
	];
}

TSharedRef<SWidget> SEnumComboBox::MakeWidgetForOption(FComboItemType InOption) const
{
	return SNew(STextBlock).Text(FText::FromString(*InOption)).Font(FontInfo);
}

void SEnumComboBox::OnSelectionChanged(FComboItemType NewValue, ESelectInfo::Type)
{
	CurrentItem = NewValue;
}

FText SEnumComboBox::GetCurrentLabel() const
{
	if (CurrentItem.IsValid())
	{
		return FText::FromString(*CurrentItem);
	}

	return LOCTEXT("InvalidComboEntryText", "<<Invalid option>>");
}

#undef LOCTEXT_NAMESPACE
