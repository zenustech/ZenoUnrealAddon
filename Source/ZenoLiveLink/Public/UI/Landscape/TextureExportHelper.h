#pragma once

class FTextureExportHelper
{
public:
	static TSharedRef<FUICommandList> GetStaticCommandList();

	static TSharedRef<FExtender> MakeExtender();

	static void BuildExportToolbar(FToolBarBuilder& Builder);

	static void Register();

	static void Unregister();

	inline static TSharedPtr<FExtender> CurrentExtender = MakeExtender();
};
