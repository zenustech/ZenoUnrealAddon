﻿#pragma once
#include "ZenoEditorMenuExtender.h"

struct FWavefrontObjectContext;

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
	void ImportVAT();

	static void ProcessObjFileImport(const FString& FilePath);
	static bool TryLoadPositionAndNormalVATBinaryDescriptor(const FString& InObjPath, UMaterialInstance* MaterialInstance);
	static bool SaveRawMeshToStaticMesh(FRawMesh& InRawMesh);
	static bool SaveContextToStaticMesh(const FWavefrontObjectContext& InContext);
};
