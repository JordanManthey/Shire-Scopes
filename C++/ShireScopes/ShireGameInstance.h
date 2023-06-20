// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AdvancedFriendsGameInstance.h"
#include "LobbyPlayerData.h"
#include "CustomGameState.h"
#include "DatabaseManager.h"
#include "ShireGameInstance.generated.h"

/**
 * The custom GameInstance class to be used by this game.
 * Responsible for persisting certain data, managing sessions, and global/debug settings.
 * Refer to BP_ShireGameInstance for more functionality.
 */

UCLASS()
class SHIRESCOPES_API UShireGameInstance : public UAdvancedFriendsGameInstance
{
	GENERATED_BODY()

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// DEBUG TOOLS
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/** Sets the number of clients to be expected when logging into a GameMode. Allows devs to test matches without having to configure a lobby. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	int Debug_TargetPlayerCount{ 1 };

	/** Sets the number of bots to generate in this level. Allows devs to test matches without having to configure a lobby. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	int Debug_TargetAICount{ 1 };

	/** Set to true if networking should be through LAN instead of SteamOnlineSubsystem. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	bool Debug_bUseLAN{ false };

	/** Set to true to prevent having inputs locked while waiting for a match to start. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	bool Debug_bPreventInputLocks{ false };

	/** Set to true to make locally controlled players invincible. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	bool Debug_bGodMode{ false };

	/** Set to true to enforces seamful travel so multiplayer can be tested in PIE (only Standalone supports seamless travel). */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	bool Debug_bPreventSeamlessTravel{ false };

	/** Set to the team value that all bots should be on. If 0 and the bots teams weren't set in a lobby, bots will alternate teams evenly. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	int Debug_ForceBotTeamAssignment{ 0 };

	/** Don't touch this. Used to cache bot team assignments. */
	UPROPERTY(BlueprintReadOnly)
	int Debug_PreviousTeamAssignment{ 2 };

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GLOBAL SETTINGS
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/** The amount of clients that can be connected to any session. */
	UPROPERTY(BlueprintReadWrite)
	int SessionMaxPlayers{ 8 };

	/** The amount of bots that can be added to any GameMode. */
	UPROPERTY(BlueprintReadWrite)
	int BotMaxPlayers{ 8 };

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// PERSISTENT DATA
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/** Used to persist data from the Lobby. Needed for seamful travel since data can't persist from the lobby to the match. */
	UPROPERTY(BlueprintReadOnly)
	TArray<FLobbyPlayerData> LobbyPlayersData;
	FLobbyPlayerData* GetLobbyPlayerData(FString PlayerName);
	FLobbyPlayerData* GetLobbyBotData(int32 BotID);
	TArray<FLobbyPlayerData> GetAllLobbyPlayersData();
	TArray<FLobbyPlayerData> GetAllLobbyBotsData();

	/** Used to determine where the player previously was before the most recent travel. */
	UClass* PreviousGameStateClass;
	UFUNCTION(BlueprintCallable)
	void CacheGameStateClass();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// SESSIONS
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/** Most session functionality is implemented in BP_ShireGameInstance. */

	/** Used to handle a client who prematurely left a session hosted by another. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void EndClientSession(APlayerController* Player);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// DATABASE
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/** 
	 * The only database manager that should be used throughout the game.
	 * Since this must be constructed in a Level, this will be set and destroyed between travels.
	 */
	UPROPERTY(BlueprintReadWrite)
	ADatabaseManager* DatabaseManager;

	/** Creates the DatabaseManager in the existing level and caches its reference. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetupDatabaseManager();

protected:

private:
	
};
