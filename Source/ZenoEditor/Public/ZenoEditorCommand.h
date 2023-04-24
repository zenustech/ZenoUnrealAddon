#pragma once

class FZenoEditorCommand : public TCommands<FZenoEditorCommand>
{
public:
	FZenoEditorCommand();
	
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> Debug;
	TSharedPtr<FUICommandInfo> ImportWavefrontMesh;
	TSharedPtr<FUICommandInfo> ImportZslFile;
	// Landscape
	TSharedPtr<FUICommandInfo> ImportHeightField;
};
