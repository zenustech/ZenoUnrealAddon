#pragma once

class FZenoTextureExportCommand final : public TCommands<FZenoTextureExportCommand>
{
public:
	FZenoTextureExportCommand();
	
	virtual void RegisterCommands() override;
	
};
