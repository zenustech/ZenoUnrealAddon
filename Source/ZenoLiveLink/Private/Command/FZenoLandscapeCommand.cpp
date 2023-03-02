#include "FZenoLandscapeCommand.h"

#include "UI/ZenoWidgeStyleWidgetStyle.h"

#define LOCTEXT_NAMESPACE "Command"

FZenoLandscapeCommand::FZenoLandscapeCommand()
	: TCommands(
		TEXT("ZenoLandscapeCommand"),
		LOCTEXT("ZenoLandscapeCommands", "Zeno landscape commands"),
		NAME_None,
		FZenoWidgetStyle::TypeName)
{
}

void FZenoLandscapeCommand::RegisterCommands()
{
	UI_COMMAND(ImportLiveLinkHeightmap, "ImportLiveLinkHeightmap", "Import heightmap from live link",
	           EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
