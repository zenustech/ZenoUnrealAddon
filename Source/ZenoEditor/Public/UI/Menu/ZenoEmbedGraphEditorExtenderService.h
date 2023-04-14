#pragma once
#include "ZenoEditorMenuExtender.h"

class FZenoEmbedGraphEditorExtenderService final : public FZenoEditorExtenderServiceBase
{
public:
	FZenoEmbedGraphEditorExtenderService();
	
	virtual void Register() override;
	virtual void Unregister() override;
	
	static FZenoEmbedGraphEditorExtenderService& Get();

private:
	void ExtendTopMenuBar(FMenuBarBuilder& Builder);
	void ExtendEmbedPullDownMenu(FMenuBuilder& Builder);

protected:
	virtual void MapAction() override;

	// Actions
	void Debug();
	void ImportZslFile();
};
