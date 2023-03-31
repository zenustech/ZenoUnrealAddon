// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZenoEngine.h"
#include "Core.h"
#include "DesktopPlatformModule.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FZenoEngineModule"

void FZenoEngineModule::StartupModule()
{
	FString BaseDir = IPluginManager::Get().FindPlugin("ZenoEngine")->GetBaseDir();

}

void FZenoEngineModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FZenoEngineModule, ZenoEngine)
