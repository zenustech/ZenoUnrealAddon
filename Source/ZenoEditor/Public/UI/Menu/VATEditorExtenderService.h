#pragma once
#include "ZenoEditorMenuExtender.h"

class FVATEditorExtenderService final : public FZenoEditorExtenderServiceBase, public TGetFromThis<FVATEditorExtenderService>
{
public:
	FVATEditorExtenderService();
	
	virtual void Register() override;
	virtual void Unregister() override;

	void ExtendMenuBar(FMenuBarBuilder& Builder);
	void ExtendVATPullDownMenu(FMenuBuilder& Builder);
	virtual void MapAction() override;

private:
	void Debug();
	void ImportWavefrontObjFile();

	static void ProcessObjFileImport(const FString& FilePath);
	static bool TryLoadPositionAndNormalVATBinaryDescriptor(const FString& InObjPath, UMaterialInstance* MaterialInstance);
};
