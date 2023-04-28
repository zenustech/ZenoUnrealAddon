using System.IO;
using UnrealBuildTool;

public class ZenoLiveLinkRuntime : ModuleRules
{
    public ZenoLiveLinkRuntime(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        bUseRTTI = true;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "InputCore",
                "HTTP",
                "Foliage",
                "Landscape",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Engine",
                "Slate",
                "SlateCore",
                "LiveLinkInterface",
            }
        );
        
        
		if (Target.bBuildEditor == true)
		{
			 PrivateDependencyModuleNames.AddRange(
				  new string[]
				  {
					  "UnrealEd",
					  "Settings",
					  "SourceControl",
				  }
			 );
		}
        // For shared header (ZenoRemoteTypes.h)
        // Pay attention, this module shouldn't use exported symbols from Zeno.
        PublicSystemIncludePaths.AddRange(new string[]
        {
			Path.Join(PluginDirectory, "Source/ThirdParty/Zeno/projects/UnrealTool/include"),
        });
		// CMakeTarget.add(Target, this, "zeno", "../ThirdParty/Zeno", "-DCMAKE_BUILD_TYPE=Release -DZENO_MULTIPROCESS:BOOL=OFF -DZENO_IPC_USE_TCP:BOOL=OFF -DZENO_BUILD_EDITOR=OFF -DZENO_MULTIPROCESS=OFF -DZENO_SYSTEM_OPENVDB=OFF -DZENO_SYSTEM_ALEMBIC=OFF -DZENO_WITH_zenvdb:BOOL=ON -DZENO_WITH_ZenoFX:BOOL=ON -DZENO_WITH_UnrealTool:BOOL=ON");
    }
}