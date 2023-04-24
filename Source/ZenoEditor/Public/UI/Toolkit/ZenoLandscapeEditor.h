#pragma once

#include "CoreMinimal.h"
#include "ZenoEditorToolkitBase.h"
#include "Blueprint/Panel/ZenoLandscapeObject.h"
#include "ZenoLandscapeEditor.generated.h"

UCLASS()
class ZENOEDITOR_API UZenoLandscapeEditor : public UZenoEditorToolkitBase
{
	GENERATED_BODY()

public:
	UZenoLandscapeEditor();
	
	virtual FText GetDisplayName() const override;
	virtual bool CanBeCreate(const FSpawnTabArgs& Args) const override;
	virtual FName GetUniqueName() const override;
	virtual void Init() override;
	virtual void Register() override;

	void OnEditorModeChanged(const FEditorModeID& InModeID, bool bIsEnteringMode);

private:
	UPROPERTY(VisibleAnywhere, Category = Zeno)
	UZenoLandscapeObject* State;
};
