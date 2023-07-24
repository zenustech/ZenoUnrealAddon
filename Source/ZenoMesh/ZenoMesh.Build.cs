using UnrealBuildTool;

public class ZenoMesh : ModuleRules
{
    public ZenoMesh(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "Engine",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "RenderCore",
                "Projects",
                "RHI",
                "RHICore",
                "ZenoEngine",
            }
        );

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "ViewportInteraction",
                    "ComponentVisualizers",
                }
            );
        }
        
        #if UE_5_2_OR_LATER
        PublicDefinitions.AddRange(new string[]
        {
            "UE_5_2_OR_LATER=1",
        });
        #endif
    }
}