using UnrealBuildTool;

public class ZenoLibrary : ModuleRules
{
    public ZenoLibrary(ReadOnlyTargetRules Target) : base(Target)
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
                "ProceduralMeshComponent",
                "Foliage",
				"Landscape",
                "LandscapeEditorUtilities",
                "RenderCore",
                "RHI",
                "InputCore",
				"ImageCore",
				"EditorFramework",
            }
        );

    }
}