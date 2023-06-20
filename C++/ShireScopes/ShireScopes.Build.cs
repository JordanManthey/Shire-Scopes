// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ShireScopes : ModuleRules
{
	public ShireScopes(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "UMG", "ActorSequence", "MovieScene", "Niagara", "OnlineSubsystem", "AIModule", "GameplayTasks", "OnlineSubsystemUtils", "Steamworks" });
	}
}
