// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class ZenoEngine : ModuleRules
{
	public ZenoEngine(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Engine",
				"Slate",
				"SlateCore",
				"CoreUObject",
				"RawMesh",
				"Renderer",
				"RenderCore",
				"InputCore",
				"Projects",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
		
        #if UE_5_2_OR_LATER
        PublicDefinitions.AddRange(new string[]
        {
            "UE_5_2_OR_LATER=1",
        });
        #endif
		
        PublicSystemIncludePaths.AddRange(new string[]
        {
			Path.Join(PluginDirectory, "Source/ThirdParty/Zeno/projects/UnrealTool/include"),
        });
        
	}
}
