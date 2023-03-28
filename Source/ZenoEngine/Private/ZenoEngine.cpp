// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZenoEngine.h"
#include "Core.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "Engine/Engine.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Importer/VAT/VATTypes.h"
#include "Importer/VAT/VATUtility.h"
// PRAGMA_PUSH_PLATFORM_DEFAULT_PACKING
// THIRD_PARTY_INCLUDES_START
// #include "zeno/core/Session.h"
// #include "zeno/core/Graph.h"
// #include "zeno/core/INode.h"
// #include "zeno/DictObject.h"
// THIRD_PARTY_INCLUDES_END
// PRAGMA_POP_PLATFORM_DEFAULT_PACKING

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
