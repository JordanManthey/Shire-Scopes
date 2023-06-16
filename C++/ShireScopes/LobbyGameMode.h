// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomGameMode.h"
#include "LobbyGameMode.generated.h"

class ALobbyGameState;

/**
 * The base GameMode used for hosting sessions and managing the pre-match lobby of players and their data.
 * Used for custom lobbies and public matchmaking.
 * Refer to BP_LobbyGameMode for more functionality.
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerJoinOrLeave, bool, bAllPlayersReady);

UCLASS()
class SHIRESCOPES_API ALobbyGameMode : public ACustomGameMode
{
	GENERATED_BODY()
	
public:

	/** Triggered when a player leaves or joins the Lobby. */
	UPROPERTY(BlueprintAssignable)
	FOnPlayerJoinOrLeave OnPlayerJoinOrLeave;

	/** The maximum team partitions this Lobby can have. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int MaxTeams{ 1 };

	/** 
	 * The maximum team partitions this Lobby can have. 
	 * @param LevelName the name of the Level to travel to.
	 * @param GameModeName the name of the GameMode to use for the next Level. By default it is the Level's default GameMode.
	 */
	UFUNCTION(BlueprintCallable)
	void StartGame(const FString& LevelName, const FString& GameModeName = FString(TEXT("")));

	/** Handle a new bot added to the Lobby. */
	UFUNCTION(BlueprintCallable)
	void HandleStartingNewBot();

	/** 
	 * Remove a bot from the lobby.
	 * @param BotID the PlayerID of the bot controller that should be removed.
	 */
	UFUNCTION(BlueprintCallable)
	void RemoveBot(int32 BotID);

	ALobbyGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void SwapPlayerControllers(APlayerController* OldPC, APlayerController* NewPC) override;

protected:

	virtual void BeginPlay() override;

	/** Caches the Lobby's player data to the GameInstance so it can persist across both seamful and seamless travel. */
	void PersistLobbyPlayersData();

private:

};
