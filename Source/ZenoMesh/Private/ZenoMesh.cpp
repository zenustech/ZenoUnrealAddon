#include "ZenoMesh.h"

#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FZenoMeshModule"

void FZenoMeshModule::StartupModule()
{
	const FString PluginDir = IPluginManager::Get().FindPlugin("ZenoEngine")->GetBaseDir();
	const FString ShaderDir = FPaths::Combine(PluginDir, TEXT("Shaders/ZenoMesh"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/ZenoMesh"), ShaderDir);
}

void FZenoMeshModule::ShutdownModule()
{
}

DEFINE_LOG_CATEGORY(LogZenoMesh);

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FZenoMeshModule, ZenoMesh)