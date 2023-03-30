using UnrealBuildTool;

public class ZenoEditor : ModuleRules
{
    public ZenoEditor(ReadOnlyTargetRules Target) : base(Target)
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
                "UnrealEd",
                "SlateCore",
                "ZenoEngine",
                "LevelEditor",
                "DesktopPlatform",
                "MeshDescription",
                "StaticMeshDescription",
                "ProceduralMeshComponent",
                "ProceduralMeshComponentEditor",
            }
        );
    }
}