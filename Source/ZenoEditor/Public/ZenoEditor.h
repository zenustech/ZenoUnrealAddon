#pragma once

#include "CoreMinimal.h"
#include "UObject/GCObjectScopeGuard.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(ZenoEditor, Log, All);

class FZenoEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    void RegisterActorFactory() const;
    
    void RegisterDetailPanelCustomization();
    void UnregisterDetailPanelCustomization();

protected:
    TSharedPtr<FGCObjectScopeGuard> LandscapeEditorGuard;
};
