#pragma once

class FZenoLandscapeCommand : public TCommands<FZenoLandscapeCommand>
{
public:
	FZenoLandscapeCommand();

	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> ImportLiveLinkHeightmap;
	TSharedPtr<FUICommandInfo> ExportLiveLinkWeightmap;

	TMap<FName, TSharedPtr<FUICommandInfo>> NameToCommandMap;
};
