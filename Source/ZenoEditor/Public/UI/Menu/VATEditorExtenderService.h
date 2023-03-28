#pragma once
#include "ZenoEditorMenuExtender.h"

class FVATEditorExtenderService final : public IZenoEditorExtenderService
{
public:
	FVATEditorExtenderService();
	
	virtual void Register() override;
	virtual void Unregister() override;

	void ExtendMenuBar(FMenuBarBuilder& Builder);
	void MapAction();
	
	static FVATEditorExtenderService& Get();

private:
	TSharedPtr<FExtender> MenuBarExtender;
	TSharedPtr<FUICommandList> CommandList;

private:
	void Debug();
};
