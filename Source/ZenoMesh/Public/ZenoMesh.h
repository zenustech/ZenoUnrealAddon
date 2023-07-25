#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "ZenoMeshCommon.h"

class FZenoMeshModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
