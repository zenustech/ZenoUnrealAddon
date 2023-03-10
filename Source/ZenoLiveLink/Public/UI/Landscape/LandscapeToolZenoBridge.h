#pragma once
#include "UI/ZenoCommonDataSource.h"
#include "UI/ZenoEditorToolkit.h"
#include "LandscapeToolZenoBridge.generated.h"

class UZenoLandscapeEditorObject;
UCLASS()
class ZENOLIVELINK_API UZenoLandscapeTool : public UZenoEditorToolkit
{
	GENERATED_BODY()
public:
	UZenoLandscapeTool(const FObjectInitializer& ObjectInitializer);

	virtual void Init() override;
	virtual bool CanBeCreate(const FSpawnTabArgs& Args) override;

	bool GetIsPropertyVisible(const FPropertyAndParent& PropertyAndParent) const;

protected:
	void Slate_MarkImportHeightfield(FToolBarBuilder& ToolBarBuilder, const TSharedRef<SVerticalBox> Container);
	void Slate_MarkExportWeightmap(FToolBarBuilder& ToolBarBuilder, const TSharedRef<SVerticalBox> Container);
	void Slate_VisualLayer(FToolBarBuilder& ToolBarBuilder, const TSharedRef<SVerticalBox> Container);
	
private:
	bool CheckSubjectKey() const;
	
	void ImportHeightMapFromSubject();
	void SaveHeightMapToAsset();
	void ReimportHeightMapToSelectedLandscape();

#pragma region FetchState
#pragma endregion FetchState
	
private:
#pragma region State
	// UI
	TSharedPtr<class SScrollBox> Slate_SubjectListView;
	TSharedPtr<class SComboButton> Slate_SubjectListComboButton;
	TSharedPtr<class IDetailsView> Slate_DetailPanel;

	UPROPERTY()
	UZenoLandscapeEditorObject* UISetting;
#pragma endregion State

#pragma region EventHandler
	FOnClicked CreateOnSubjectListItemClicked(const FLiveLinkSubjectKey& Key);
#pragma endregion EventHandler

public:
	inline static const FName TabName = FName("ZT_Landscape");

	inline static const FName NAME_ImportHeightfield = FName("ZT_Landscape_ImportHeightfield");
	inline static const FName NAME_ExportWeightmap = FName("ZT_Landscape_ExportWeightmap");
	inline static const FName NAME_VisualLandscapeLayer = FName("ZT_Landscape_VisualLandscapeLayer");

	static void SetSharedCurrentMode(const FName InName);

	inline static FName LandscapeToolCurrentMode = NAME_None;
};
