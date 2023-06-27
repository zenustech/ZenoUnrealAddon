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
	UI_COMMAND(ZenoGraphHeader, "Zeno Graph", "Zeno remote graph tool", EUserInterfaceActionType::Button, {});
	UI_COMMAND(Debug, "Debug", "Debug", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ImportWavefrontMesh, "Import .obj", "Import .obj file without reduction and full uv precision.", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ImportVAT, "Import VAT", "Open VAT import panel.", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ImportZslFile, "Import ZSL", "Import json action dump file exported by zeno.", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ImportHeightField, "Import", "Import height field subject from zeno.", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ExportHeightField, "Export", "Export height field data to zeno.", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
