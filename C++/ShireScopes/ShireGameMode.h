// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CustomGameMode.h"
#include "MatchTeamData.h"
#include "ShireGameMode.generated.h"

class AShireCharacter;
class AShireAIController;
class APlayerStart;

struct FShireGameMode_SetupData : FCustomGameMode_SetupData
{
	bool bAllLocalCharactersReady_Complete{ false };
};

/**
 * This is the base class for all match-based GameModes in which players/bots control a ShireCharacter to play.
 */

UCLASS(Abstract, minimalapi)
class AShireGameMode : public ACustomGameMode
{
	GENERATED_BODY()

public:

	/** The delay for spawning a player if its respawn is queued. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "General")
	float SpawnDelay{ 3.0 };

	/** This struct is used to maintain data for each team in the match. */
	TMap<int, FMatchTeamData> MatchTeamsData;

	AShireGameMode();
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	/** Triggered when a character is possessed by a local controller and has begun play. */
	virtual void OnLocalCharacterPossessAndBeginPlay();

protected:

	/** Used to flag various setup processes as completed. Useful for tracking GameMode setup and enabling tick queries. */
	FShireGameMode_SetupData SetupData;
	
	/** The AIController class to be used for bots. */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TSubclassOf<AShireAIController> BotControllerClass;

	/** An array of all bot controllers present in this match. */
	TArray<AShireAIController*> BotControllers;

	/** The Character class to be used for this match. */
	UClass* ShirePawnClass;

	/** The number of rounds to be played in this match. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "General")
	int MaxRound{ 1 };
	int CurrentRound{ 0 };

	/** The amount of characters that have been possessed by a local controller and begun play. */
	int LocalCharacterReadyCount{ 0 };

	/** The amount of players that have logged into this GameMode. */
	int PlayerLoginCount{ 0 };

	/** A map from team to array of spawn locations that the team is using. */
	TMap<int, TArray<APlayerStart*>> TeamToSpawnMap;
	TArray<APlayerStart*> OccupiedSpawnPoints;
	TArray<APlayerStart*> UnoccupiedSpawnPoints;

	/** 
	 * Queues a player/bot to be spawned in SpawnDelay time. 
	 * @param Player the player/bot to spawn and possess a new character.
	 */
	UFUNCTION()
	void QueuePlayerSpawn(AController* Player);

	/**
	 * Instantly spawns a player/bot.
	 * @param Player the player/bot to spawn and possess a new character.
	 */
	UFUNCTION()
	virtual void SpawnPlayer(AController* Player);

	/** Triggered when a player/bot dies. */
	UFUNCTION()
	virtual void OnPlayerDeath(AShireCharacter* DeathCharacter);

	/** Brings all clients in this match back to the lobby. */
	UFUNCTION(BlueprintCallable)
	virtual void ExitMatch();
	
	/** Handles starting a round */
	UFUNCTION()
	virtual void StartRound();

	/** Handles ending a round */
	virtual void EndRound();

	/** Handles ending the entire match */
	virtual void EndMatch();

	virtual void BeginPlay() override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void SetMaxPlayers() override;
	virtual void OnAllLocalControllersReady() override;

	/** @returns the team that won the entire match. */
	virtual int GetMatchWinningTeam() const;

	/** @returns the team that won the current round. */
	virtual int GetRoundWinningTeam() const;

	/** @returns true if any team has met the criteria for winning the entire match. */
	virtual bool AnyTeamHasWonMatch() const;

	/** @returns true if any team has met the criteria for winning the current round. */
	virtual bool AnyTeamHasWonRound() const;

	/** Triggered when all players that were expected from the lobby have logged into this GameMode. */
	virtual void OnAllPlayersLogin();

	/** Triggered when all expected locally controlled players have possessed a character and their character has begun play. */
	virtual void OnAllLocalCharactersReady();

	/** Triggered when both OnAllLocalCharactersReady() and OnAllLocalControllersReady() have been triggered. */
	virtual void OnAllLocalControllersAndCharactersReady();

	/** Resets the spawn points so that none are considered occupied and no team has spawn points allocated. */
	void ResetSpawnPoints();

	/** Spawn all players with a possessed character. */
	void SpawnAllPlayers();

	/** Spawn all bots with a possessed character. */
	void SpawnAllBots();

	/** Setup all bots. */
	void SetupAllBots();

	/** @returns true if every player on @param TargetTeam is dead. */
	bool IsTeamDead(int TargetTeam) const;

	/** Sends the match results to the DatabaseManager to update the leaderboard. */
	void UpdateLeaderboard();

private:

};



