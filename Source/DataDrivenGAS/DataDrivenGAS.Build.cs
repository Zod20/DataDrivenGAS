// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DataDrivenGAS : ModuleRules
{
	public DataDrivenGAS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay",
                    "GameplayAbilities", "GameplayTags","GameplayTasks"                                                                     //GAS combat modules
		});
	}
}
