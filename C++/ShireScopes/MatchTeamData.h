// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MatchTeamData.generated.h"

/**
 * A struct used by ShireGameMode to maintain data for each team during a match.
 */

USTRUCT(BlueprintType)
struct FMatchTeamData {

	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite)
	int Team{ 1 };

	UPROPERTY(BlueprintReadWrite)
	int RoundsWon{ 0 };

};
