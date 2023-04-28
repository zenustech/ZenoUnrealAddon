#pragma once
#include "ZenoEditorMenuExtender.h"

class FZenoRemoteGraphMenuExtender : public FZenoEditorExtenderServiceBase, public TGetFromThis<FZenoRemoteGraphMenuExtender>
{
public:
	virtual void Register() override;
	virtual void Unregister() override;

protected:
	virtual void MapAction() override;

	void ExtendTopMenuBar(FMenuBarBuilder& Builder);
	void ExtendEmbedPullDownMenu(FMenuBuilder& Builder);

private:
	void Debug();
	void ImportZslFile();
};
