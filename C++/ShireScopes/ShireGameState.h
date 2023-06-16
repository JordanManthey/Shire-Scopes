// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomGameState.h"
#include "ShireGameMode.h"
#include "MatchPlayerData.h"
#include "ShireGameState.generated.h"

/**
 * This is the base class for all match-based GameStates in which players/bots control a ShireCharacter to play.
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoundCountdown);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoundStart);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDataUpdate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllLocalCharactersReady);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundOver, int, WinningTeam);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchOver, int, WinningTeam);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLocalPlayerSet, APlayerController*, LocalPlayer);

UCLASS()
class SHIRESCOPES_API AShireGameState : public ACustomGameState
{
	GENERATED_BODY()

public:

	AShireGameState();

	/** Triggered when all characters that are possessed by a local controller have begun play. */
	UPROPERTY(BlueprintAssignable)
	FOnAllLocalCharactersReady OnAllLocalCharactersReady;

	/** Triggered when this game clients LocalPlayer has been set. */
	UPROPERTY(BlueprintAssignable)
	FOnLocalPlayerSet OnLocalPlayerSet;

	/** Triggered when an entry in MatchPlayersData has been changed. */
	UPROPERTY(BlueprintAssignable)
	FOnPlayerDataUpdate OnPlayerDataUpdate;

	/** Triggered when the round has commenced its countdown to start. */
	UPROPERTY(BlueprintAssignable)
	FOnRoundCountdown OnRoundCountdown;

	/** Triggered when the round has started. */
	UPROPERTY(BlueprintAssignable)
	FOnRoundStart OnRoundStart;

	/** Triggered when the round has ended. */
	UPROPERTY(BlueprintAssignable)
	FOnRoundOver OnRoundOver;

	/** Triggered when the entire match has ended. */
	UPROPERTY(BlueprintAssignable)
	FOnRoundOver OnMatchOver;

	/** The time countdown before starting a round. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "General")
	int StartRoundDelay{ 10 };

	/** The time delay between a round finishing and commencing the following round's countdown. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "General")
	float RoundTransitionDelay{ 5 };

	/** The time delay between a match finishing and the match being exited. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "General")
	float ExitMatchTransitionDelay{ 10 };

	/** An array of MatchPlayerData used to track the stats of each player/bot throughout the match. */
	UPROPERTY(ReplicatedUsing = OnRep_PlayerDataUpdate, BlueprintReadOnly)
	TArray<FMatchPlayerData> MatchPlayersData;

	/** The local controller on this game client. */
	UPROPERTY(BlueprintReadOnly)
	APlayerController* LocalPlayer;

	/** Triggered when a player/bot takes damage. */
	UFUNCTION()
	void OnPlayerTakeDamage(AController* DamageReceiver, AController* DamageDealer, float DamageAmount);

	/** Triggered when a player/bot receives healing. */
	UFUNCTION()
	void OnPlayerTakeHealing(AController* HealingReceiver, AController* HealingDealer, float HealingAmount);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartRoundCountdown();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnAllLocalCharactersReady();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnRoundOver(int WinningTeam);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnMatchOver(int WinningTeam);

	void AddPlayerState(APlayerState* PlayerState) override;
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnPlayerLogout(APlayerController* Player) override;
	void SetLocalPlayer(APlayerController* TargetPlayer);
	void AddMatchPlayerData(AController* NewPlayer);
	void UpdateMatchPlayerHealing(int32 PlayerID, int DeltaHealing);
	void UpdateMatchPlayerDamage(int32 PlayerID, int DeltaDamage);
	void UpdateMatchPlayerKills(int32 PlayerID, int DeltaKills);
	void UpdateMatchPlayerDeaths(int32 PlayerID, int DeltaDeaths);
	void BindToCharacterEvents(AShireCharacter* ShireCharacter);
	FMatchPlayerData* GetMatchPlayerData(int32 PlayerID);

protected:

	UFUNCTION()
	void StartRound();

	UFUNCTION()
	void OnRep_PlayerDataUpdate();

	virtual void BeginPlay() override;
	virtual void OnRep_ReplicatedHasBegunPlay() override;
	virtual void RemoveMatchPlayerData(APlayerController* Player);

private:

};
