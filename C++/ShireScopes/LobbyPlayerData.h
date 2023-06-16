// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LobbyPlayerData.generated.h"

/**
 * A struct used by LobbyGameState to maintain data for each player and bot.
 */

USTRUCT(BlueprintType)
struct FLobbyPlayerData {

	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly)
	int32 PlayerID;

	UPROPERTY(BlueprintReadWrite)
	int Team{ 1 };

	UPROPERTY(BlueprintReadWrite)
	bool ReadyStatus{ false };

};
