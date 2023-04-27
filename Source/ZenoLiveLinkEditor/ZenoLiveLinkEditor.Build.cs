using UnrealBuildTool;

public class ZenoLiveLinkEditor : ModuleRules
{
    public ZenoLiveLinkEditor(ReadOnlyTargetRules Target) : base(Target)
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
                "ZenoLiveLinkRuntime",
                "ZenoEditor",
                "UnrealEd",
                "Foliage",
                "Landscape",
                "LandscapeEditor",
                "LiveLinkInterface",
            }
        );
    }
}