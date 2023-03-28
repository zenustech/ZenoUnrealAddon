#pragma once

class FZenoEditorCommand : public TCommands<FZenoEditorCommand>
{
public:
	FZenoEditorCommand();

	TSharedPtr<FUICommandInfo> Debug;
	
	virtual void RegisterCommands() override;
protected:
};
