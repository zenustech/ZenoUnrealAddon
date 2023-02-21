#include "ZenoLiveLink.h"

#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FZenoLiveLinkModule"

void FZenoLiveLinkModule::StartupModule()
{
	// Module = &FModuleManager::Get().GetModuleChecked<FZenoBridgeModule>("ZenoBridge");
}

void FZenoLiveLinkModule::ShutdownModule()
{
    Module = nullptr;
}

void FZenoLiveLinkModule::Tick(float DeltaTime)
{
}

bool FZenoLiveLinkModule::IsTickable() const
{
	return true;
}

TStatId FZenoLiveLinkModule::GetStatId() const
{
	return {};
}

IMPLEMENT_MODULE(FZenoLiveLinkModule, ZenoLiveLink)
#undef LOCTEXT_NAMESPACE
