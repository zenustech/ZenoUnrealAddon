#pragma once

class FZenoLandscapeCommand : public TCommands<FZenoLandscapeCommand>
{
public:
	FZenoLandscapeCommand();

	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> ImportLiveLinkHeightmap;
};
