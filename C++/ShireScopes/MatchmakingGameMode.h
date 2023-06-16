// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LobbyGameMode.h"
#include "MatchmakingGameMode.generated.h"

class UDataTable;

/**
 * The GameMode used for matchmaking.
 * Used to override functionality from LobbyGameMode.
 * Refer to BP_MatchmakingGameMode for more functionality.
 */

UCLASS()
class SHIRESCOPES_API AMatchmakingGameMode : public ALobbyGameMode
{
	GENERATED_BODY()

public:

protected:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FString GetRandomLevelName() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnStartGame();

	virtual void SetMaxPlayers() override;
	virtual void OnAllLocalControllersReady() override;

private:

};
