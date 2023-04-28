#pragma once
#include "ZenoEditorMenuExtender.h"

class FZenoEmbedGraphEditorExtenderService final : public FZenoEditorExtenderServiceBase, public TGetFromThis<FZenoEmbedGraphEditorExtenderService>
{
public:
	FZenoEmbedGraphEditorExtenderService();
	
	virtual void Register() override;
	virtual void Unregister() override;
	
private:
	void ExtendTopMenuBar(FMenuBarBuilder& Builder);
	void ExtendEmbedPullDownMenu(FMenuBuilder& Builder);

protected:
	virtual void MapAction() override;

	// Actions
	void Debug();
	void ImportZslFile();
};
