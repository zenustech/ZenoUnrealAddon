using System.IO;
using UnrealBuildTool;

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
                "Core",
                "CoreUObject",
                "ApplicationCore",
                "InputCore",
                "Json",
                "ImageWrapper",
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
                "PropertyEditor",
                "MeshUtilities",
				"DesktopPlatform",
				"UObjectPlugin",
                "ZenoLiveLinkRuntime",
                "EditorFramework",
                "LiveLinkInterface",
                "Landscape",
                "LandscapeEditor",
            }
        );
        
		// CMakeTarget.add(Target, this, "zeno", "../ThirdParty/Zeno", "-DZENO_MULTIPROCESS:BOOL=OFF -DZENO_IPC_USE_TCP:BOOL=OFF -DZENO_BUILD_EDITOR=OFF -DZENO_MULTIPROCESS=OFF -DZENO_SYSTEM_OPENVDB=OFF -DZENO_SYSTEM_ALEMBIC=OFF -DZENO_WITH_ZenoFX:BOOL=ON -DZENOFX_ENABLE_OPENVDB:BOOL=ON -DZENO_WITH_zenvdb:BOOL=ON -DZENO_WITH_Rigid:BOOL=ON -DZENO_WITH_Euler:BOOL=ON -DZENO_WITH_Functional:BOOL=ON -DZENO_WITH_CalcGeometryUV:BOOL=ON -DZENO_WITH_SampleModel:BOOL=ON -DZENO_WITH_MeshSubdiv:BOOL=ON -DZENO_WITH_CUDA:BOOL=OFF -DZENO_ENABLE_OPTIX:BOOL=OFF -DZENO_ENABLE_UnrealTool:BOOL=ON -DWINDOWS_EXPORT_ALL_SYMBOLS=ON");
		// CMakeTarget.add(Target, this, "zeno", "../ThirdParty/Zeno", "-DCMAKE_BUILD_TYPE=Release -DZENO_MULTIPROCESS:BOOL=OFF -DZENO_IPC_USE_TCP:BOOL=OFF -DZENO_BUILD_EDITOR=OFF -DZENO_MULTIPROCESS=OFF -DZENO_SYSTEM_OPENVDB=OFF -DZENO_SYSTEM_ALEMBIC=OFF -DZENO_WITH_zenvdb:BOOL=ON -DZENO_WITH_ZenoFX:BOOL=ON -DZENO_WITH_UnrealTool:BOOL=ON");
        PublicSystemIncludePaths.AddRange(new string[]
        {
			Path.Join(PluginDirectory, "Source/ThirdParty/Zeno/projects/UnrealTool/include"),
        });
    }
}