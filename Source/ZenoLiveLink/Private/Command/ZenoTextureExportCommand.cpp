#include "ZenoTextureExportCommand.h"

#include "UI/ZenoWidgeStyleWidgetStyle.h"

#define LOCTEXT_NAMESPACE "FZenoTextureExportCommand"

FZenoTextureExportCommand::FZenoTextureExportCommand()
	: TCommands(
		TEXT("ZenoTextureExportCommand"),
		LOCTEXT("ZenoTextureExportCommands", "Zeno texture export commands"),
		NAME_None,
		FZenoWidgetStyle::TypeName)
{
}

void FZenoTextureExportCommand::RegisterCommands()
{
}

#undef LOCTEXT_NAMESPACE
