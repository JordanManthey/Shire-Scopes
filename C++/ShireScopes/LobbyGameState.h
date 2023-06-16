// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomGameState.h"
#include "LobbyPlayerData.h"
#include "LobbyGameState.generated.h"

/**
 * This is the GameState class used with any instance of LobbyGameMode.
 * Communicated critical lobby data between Server and Clients.
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReadyStatusUpdate, bool, bAllPlayersReady);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLobbyUpdate);

UCLASS()
class SHIRESCOPES_API ALobbyGameState : public ACustomGameState
{
	GENERATED_BODY()

public:

	/** Triggered when any client in the Lobby marks themselves as ready/unready. Provides a boolean param stating whether all players are ready or not. */
	UPROPERTY(BlueprintAssignable)
	FOnReadyStatusUpdate OnReadyStatusUpdate;

	/** Triggered when any critical lobby data is updated (map/mode selection, player login/logouts, bots added/removed, ready status, etc). */
	UPROPERTY(BlueprintAssignable)
	FOnLobbyUpdate OnLobbyUpdate;

	/** The index of the Level currently selected by the Host. */
	UPROPERTY(ReplicatedUsing = OnRep_LobbyUpdate, BlueprintReadWrite)
	int MapIndex;

	/** The index of the GameMode currently selected by the Host. */
	UPROPERTY(ReplicatedUsing = OnRep_LobbyUpdate, BlueprintReadWrite)
	int ModeIndex;
	
	/** The number of bots that have been added by the Host. */
	UPROPERTY(ReplicatedUsing = OnRep_LobbyUpdate, BlueprintReadWrite)
	int BotCount;

	/** An array of player data for each player and bot in the lobby. */
	UPROPERTY(ReplicatedUsing = OnRep_LobbyUpdate, BlueprintReadWrite)
	TArray<FLobbyPlayerData> LobbyPlayersData;

	/** 
	 * Update the team of a player or bot in the lobby.
	 * @param PlayerID the controller ID of the player or bot you want to update.
	 * @param NewTeam the target team to be set.
	 */
	UFUNCTION(BlueprintCallable)
	void UpdateLobbyPlayerTeam(int32 PlayerID, int NewTeam);

	/** 
	 * Update the ready status of a player or bot in the lobby (though bots are always assumed ready).
	 * @param PlayerID the controller ID of the player or bot you want to update.
	 * @param NewReadyStatus true if ready, false if not.
	 */
	UFUNCTION(BlueprintCallable)
	void UpdateLobbyPlayerReadyStatus(int32 PlayerID, bool NewReadyStatus);
	 
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnPlayerLogout(APlayerController* Player) override;

	/** 
	 * Generate a LobbyPlayerData struct from a PlayerController add it to the LobbyPlayersData array.
	 * @param NewPlayer the player that this LobbyPlayerData struct should generate from.
	 */
	void AddLobbyPlayerData(APlayerController* NewPlayer);

	/** Create a new LobbyPlayerData struct for a bot add it to the LobbyPlayersData array. */
	void AddLobbyBotData();

	FLobbyPlayerData* GetLobbyPlayerData(int32 PlayerID);
	void RemoveLobbyBotData(int32 BotID);
	virtual void RemoveLobbyPlayerData(APlayerController* Player);

	/** Save each player's team from their LobbyPlayerData to their respective PlayerState so that the team configuration can persist through seamless travel. */
	void CacheTeamDataInPlayerStates();
	
	/** @returns if all players in the Lobby have marked themselves as ready. */
	bool GetAllPlayersReady() const;

protected:

	/** Triggered when any of the LobbyGameState replicated variables are updated. */
	UFUNCTION(BlueprintCallable)
	void OnRep_LobbyUpdate(); // Triggers on Clients only, not Server.

	virtual void BeginPlay() override;

private:

	/** @returns a BotID generated based on the number of existing bots. */
	int GenerateBotID() const;

};
