#pragma once

class FZenoEditorCommand : public TCommands<FZenoEditorCommand>
{
public:
	FZenoEditorCommand();
	
	virtual void RegisterCommands() override;

public:

	// Infos
	TSharedPtr<FUICommandInfo> ZenoGraphHeader;
	
	// Debug
	TSharedPtr<FUICommandInfo> Debug;
	// VAT
	TSharedPtr<FUICommandInfo> ImportWavefrontMesh;
	TSharedPtr<FUICommandInfo> ImportVAT;
	// ZSL
	TSharedPtr<FUICommandInfo> ImportZslFile;
	// Landscape
	TSharedPtr<FUICommandInfo> ImportHeightField;
	TSharedPtr<FUICommandInfo> ExportHeightField;
};
