// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MatchPlayerData.generated.h"

/**
 * A struct used by ShireGameState to maintain data for each player and bot during a match.
 */

USTRUCT(BlueprintType)
struct FMatchPlayerData {

	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString PlayerName { "DefaultPlayerName" };

	UPROPERTY(BlueprintReadOnly)
	int32 PlayerID;

	UPROPERTY(BlueprintReadWrite)
	int Team{ 1 };

	UPROPERTY(BlueprintReadWrite)
	int KillCount{ 0 };

	UPROPERTY(BlueprintReadWrite)
	int DeathCount{ 0 };

	UPROPERTY(BlueprintReadWrite)
	int DamageCount{ 0 };

	UPROPERTY(BlueprintReadWrite)
	int HealingCount{ 0 };

};
