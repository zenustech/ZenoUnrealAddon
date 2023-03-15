using UnrealBuildTool;

public class ZenoLiveLink : ModuleRules
{
    public ZenoLiveLink(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

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
                "SlateCore",
                "PropertyEditor",
                "LiveLinkInterface",
                "ZenoBridge",
                "Landscape",
                "LandscapeEditor",
                "LevelEditor",
                "UnrealEd",
                "ZenoLibrary",
                "AssetTools",
                "EditorFramework",
                "LandscapeEditor",
                "EditorScriptingUtilities",
                "TextureEditor",
                "Foliage",
                "InputCore",
            }
        );
        
        PrivateDefinitions.AddRange(
	        new string[]
	        {
	        }
		);
    }
}