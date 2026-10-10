using UnrealBuildTool;
public class ACEConversationMetaHuman : ModuleRules
{
    public ACEConversationMetaHuman(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "ACEConversation", "ACERuntime", "ACECore" });
    }
}
