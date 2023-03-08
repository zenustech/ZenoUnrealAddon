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
	
private:
	void ImportHeightMapFromSubject();

#pragma region FetchState
	FText GetSubjectComboButtonText() const;
#pragma endregion FetchState
	
private:
#pragma region State
	// UI
	TSharedPtr<class SScrollBox> Slate_SubjectListView;
	TSharedPtr<class SComboButton> Slate_SubjectListComboButton;

	UPROPERTY()
	UZenoLandscapeEditorObject* UISetting;
#pragma endregion State

#pragma region EventHandler
	FOnClicked CreateOnSubjectListItemClicked(const FLiveLinkSubjectKey& Key);
#pragma endregion EventHandler

public:
	inline static const FName TabName = FName("ZT_Landscape");

private:
};
