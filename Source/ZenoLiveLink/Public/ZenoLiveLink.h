#pragma once

#include "CoreMinimal.h"

class UZenoLandscapeTool;

class FZenoLiveLinkModule : public IModuleInterface, public FTickableGameObject
{
public:
	// ~Begin override IModuleInterface members
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
	// ~End override IModuleInterface members
	
	// ~Begin override FTickableGameObject members
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;
	// ~End override FTickableGameObject members

	TWeakObjectPtr<UZenoLandscapeTool> GetLandscapeTool() const;

	static FZenoLiveLinkModule* GetInstance();

private:
	TSharedPtr<FUICommandList> PluginCommands;

	TStrongObjectPtr<UZenoLandscapeTool> LandscapeTool;

	inline static FZenoLiveLinkModule* ModuleInstance = nullptr;

private:
	void MapPluginActions() const;

	void OnEditorModeChanged(const FEditorModeID& InModeID, bool bIsEnteringMode);

public:
	friend UZenoLandscapeTool;
};
