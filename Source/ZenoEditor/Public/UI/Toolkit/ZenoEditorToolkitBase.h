#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ZenoEditorToolkitBase.generated.h"

UCLASS(Abstract)
class ZENOEDITOR_API UZenoEditorToolkitBase : public UObject
{
	GENERATED_BODY()

public:
	UZenoEditorToolkitBase();
	virtual ~UZenoEditorToolkitBase() override = default;

	/** Provide title of the toolkit. */
	virtual FText GetDisplayName() const;

	// Chain build section
	virtual UZenoEditorToolkitBase& AddSubMode(const FName& InModeName, const TSharedRef<FUICommandInfo> CommandInfo);

	virtual UZenoEditorToolkitBase& SetPropertyObject(UObject* Object);
	// Chain build end

	virtual FName GetCurrentMode() const;

	virtual bool IsInSubMode(const FName InModeName) const;

	// Property detail panel customization
	virtual bool CheckPropertyVisibility(const FPropertyAndParent& InPropertyAndParent) const;

	/**
	 * Tell tab manager if should create tab of toolkit.
	 * Override this function to make dock tab visible.
	 * @return boolean
	 */
	virtual bool CanBeCreate(const FSpawnTabArgs& Args) const;
	virtual FName GetUniqueName() const;

	TSharedRef<SDockTab> GetDockTab(const FSpawnTabArgs& Args);
	virtual void MakeDockTab();

	virtual bool IsSubModeEnable(const FName InModeName);
	virtual void OnChangeMode(const FName InModeName);

	virtual void Register();
	virtual void Unregister();

	/**
	 * @brief Called on register
	 */
	virtual void Init();

protected:
	TMap<FName, TSharedRef<FUICommandInfo>> SubModes;
	FName CurrentMode;

	UPROPERTY(VisibleAnywhere, Category = Zeno)
	UObject* PropertyObject;

	TSharedPtr<SDockTab> Slate_ToolkitDockTab;

	TSharedRef<FUICommandList> UICommandList;

	TSharedPtr<class IDetailsView> Slate_DetailPanel;
};
