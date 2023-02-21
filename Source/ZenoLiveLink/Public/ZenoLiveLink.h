#pragma once

#include "CoreMinimal.h"
#include "ZenoBridge.h"

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

private:
	FZenoBridgeModule* Module = nullptr;
};
