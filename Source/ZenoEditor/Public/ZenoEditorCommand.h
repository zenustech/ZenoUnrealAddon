#pragma once

class FZenoEditorCommand : public TCommands<FZenoEditorCommand>
{
public:
	FZenoEditorCommand();
	
	virtual void RegisterCommands() override;

public:

	// Infos
	TSharedPtr<FUICommandInfo> ZenoGraphHeader;
	
	//
	TSharedPtr<FUICommandInfo> Debug;
	TSharedPtr<FUICommandInfo> ImportWavefrontMesh;
	TSharedPtr<FUICommandInfo> ImportZslFile;
	// Landscape
	TSharedPtr<FUICommandInfo> ImportHeightField;
};
