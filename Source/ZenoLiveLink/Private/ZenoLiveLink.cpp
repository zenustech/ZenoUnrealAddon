#include "ZenoLiveLink.h"

#include "ZenoBridge.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FZenoLiveLinkModule"

void FZenoLiveLinkModule::StartupModule()
{
}

void FZenoLiveLinkModule::ShutdownModule()
{
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
