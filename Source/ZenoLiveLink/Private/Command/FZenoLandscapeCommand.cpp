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
	UI_COMMAND(ImportLiveLinkHeightmap, "Import - Heightmap", "Import heightmap from live link",
	           EUserInterfaceActionType::RadioButton, FInputChord());
	NameToCommandMap.Add("Import_Heightmap", ImportLiveLinkHeightmap);

	UI_COMMAND(ExportLiveLinkWeightmap, "Export - Weightmap", "Export weightmap to zeno", EUserInterfaceActionType::RadioButton, FInputChord());
	NameToCommandMap.Add("Export_Weightmap", ExportLiveLinkWeightmap);
}

#undef LOCTEXT_NAMESPACE
