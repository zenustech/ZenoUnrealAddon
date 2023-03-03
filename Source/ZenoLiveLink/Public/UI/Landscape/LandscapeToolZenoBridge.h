#pragma once
#include "UI/ZenoCommonDataSource.h"
#include "LandscapeToolZenoBridge.generated.h"

UCLASS()
class ZENOLIVELINK_API UZenoLandscapeTool : public UZenoCommonDataSource
{
	GENERATED_BODY()
public:
	UZenoLandscapeTool();
	
	static void Register();
	static void UnRegister();

	void InitWidget();

private:
	TSharedRef<class SDockTab> MakeDockTab(const FSpawnTabArgs& Args);
	bool ShouldCreateDockTab(const FSpawnTabArgs& Args);

	void ImportHeightMapFromSubject();

#pragma region FetchState
	FText GetSubjectComboButtonText() const;
#pragma endregion FetchState
	
private:
#pragma region State
	// UI
	TWeakPtr<SDockTab> Slate_LandscapeToolDockTab;
	TSharedPtr<class SScrollBox> Slate_SubjectListView;
	TSharedPtr<class SComboButton> Slate_SubjectListComboButton;
	
	TOptional<FLiveLinkSubjectKey> SelectedSubjectKey;
#pragma endregion State

#pragma region EventHandler
	FOnClicked CreateOnSubjectListItemClicked(const FLiveLinkSubjectKey& Key);
#pragma endregion EventHandler

public:
	inline static const FName TabName = FName("ZenoLandscapeTool");

private:
};
