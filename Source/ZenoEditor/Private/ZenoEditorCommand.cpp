#include "ZenoEditorCommand.h"

#include "UI/Style/ZenoEditorWidgetStyle.h"

#define LOCTEXT_NAMESPACE "FZenoEditorCommand"

FZenoEditorCommand::FZenoEditorCommand()
	: TCommands(
		TEXT("ZenoEditorCommand"),
		LOCTEXT("CommandName", "Zeno Editor"),
		NAME_None,
		FZenoEditorWidgetStyle::TypeName)
{
}

void FZenoEditorCommand::RegisterCommands()
{
	UI_COMMAND(Debug, "Debug", "Debug", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
