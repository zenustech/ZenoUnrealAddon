#pragma once

class FZenoLandscapeCommand : public TCommands<FZenoLandscapeCommand>
{
public:
	FZenoLandscapeCommand();

	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> ImportLiveLinkHeightmap;
	TSharedPtr<FUICommandInfo> ExportLiveLinkWeightmap;
	TSharedPtr<FUICommandInfo> VisualLayer;

	TMap<FName, TSharedPtr<FUICommandInfo>> NameToCommandMap;
};
