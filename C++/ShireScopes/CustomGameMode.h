// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CustomGameMode.generated.h"

/**
 * This is the base abstract class for all ShireScopes GameModes non native to UE source. 
 * WARNING: Do not instantiate this class as it is conceptually abstract. This class can't be enforcibly abstract due to UE abstract actor guardrails. 
 */

struct FCustomGameMode_SetupData
{
	bool bAllLocalControllersReady_Complete{ false };
};

UCLASS()
class SHIRESCOPES_API ACustomGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	ACustomGameMode();

	/** The max players that can be logged into this GameMode (not including AI). */
	UPROPERTY(BlueprintReadWrite)
	int MaxPlayers{ 1 };

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	/** Triggered when a local player controller possesses a pawn and has also begun play. */
	virtual void OnLocalControllerPossessAndBeginPlay();

protected:

	FCustomGameMode_SetupData SetupData;

	/** The number of local controllers that have possessed pawns and begun play. */
	int LocalControllerReadyCount{ 0 };

	/** The level the Server is awaiting travel to. Cached so that server can wait until all clients have traveled before traveling itself. */
	FString PendingTravelLevelName;

	/**
	 * The main travel function used for both client and server travel. 
	 * @param LevelName the target level for travel.
	 * @param bServerTravel if this should be a server travel with all connected clients.
	 * @param LoadingMessage the message that the loading screen shows.
	 * @param Options the options to be added to the OptionsString for opening the target level.
	 */
	UFUNCTION(BlueprintCallable)
	virtual void TravelToLevel(const FString& LevelName, bool bServerTravel, const FString& LoadingMessage = TEXT(""), const FString& Options = TEXT(""));

	/** Triggered whenever the current GameState instance adds/removes from its PlayerArray */
	UFUNCTION()
	virtual void OnPlayerArrayUpdate();

	virtual void BeginPlay() override;

	/** 
	 * @param NewPlayer the player logging in.
	 * @returns whether or not the player should be accepted into this GameMode.
	 */
	virtual bool CanPlayerJoin(APlayerController* NewPlayer);

	/**
	 * Toggles the loading screen for every client connected to this GameMode.
	 * @param bIsVisible if the loading screen should be visible or hidden.
	 * @param LoadingMessage the message that the loading screen shows.
     */
	virtual void ToggleLoadingScreens(bool bIsVisible, const FString& LoadingMessage = FString()) const;

	/** For debug purposes only. Checks if the Debug config requests that we enforce seamful travel when traveling from this GameMode. */
	void CheckPreventSeamlessTravel();

	/** Tells the GameState to flag widgets for seamless travel. */
	void FlagSeamlessTravelWidgets() const;

	/** Tells the GameState to cache it's static class. */
	void CacheGameStateClass() const;

	/** Set the maximum number of players that can be connected to this GameMode. */
	virtual void SetMaxPlayers();

	/** Triggered when all expected local controllers have possessed a pawn and begun play. */
	virtual void OnAllLocalControllersReady();

private:

};
