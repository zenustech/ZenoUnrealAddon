﻿using UnrealBuildTool;

public class ZenoEditor : ModuleRules
{
    public ZenoEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		bUseRTTI = true;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "UnrealEd",
                "SlateCore",
                "ZenoEngine",
                "LevelEditor",
                "DesktopPlatform",
                "MeshDescription",
                "StaticMeshDescription",
                "ProceduralMeshComponent",
                "ProceduralMeshComponentEditor",
                "CMakeTarget",
                "AssetTools",
                "DeveloperSettings",
                "ContentBrowser",
                "Kismet",
            }
        );
        
		CMakeTarget.add(Target, this, "zeno", "../ThirdParty/Zeno", "-DZENO_MULTIPROCESS:BOOL=OFF -DZENO_IPC_USE_TCP:BOOL=OFF -DZENO_BUILD_EDITOR=OFF -DZENO_MULTIPROCESS=OFF -DZENO_SYSTEM_OPENVDB=OFF -DZENO_SYSTEM_ALEMBIC=OFF -DZENO_WITH_ZenoFX:BOOL=ON -DZENOFX_ENABLE_OPENVDB:BOOL=ON -DZENO_WITH_zenvdb:BOOL=ON -DZENO_WITH_Rigid:BOOL=ON -DZENO_WITH_Euler:BOOL=ON -DZENO_WITH_Functional:BOOL=ON -DZENO_WITH_CalcGeometryUV:BOOL=ON -DZENO_WITH_SampleModel:BOOL=ON -DZENO_WITH_MeshSubdiv:BOOL=ON -DZENO_WITH_CUDA:BOOL=OFF -DZENO_ENABLE_OPTIX:BOOL=OFF -DCMAKE_LINK_WHAT_YOU_USE=ON");
    }
}