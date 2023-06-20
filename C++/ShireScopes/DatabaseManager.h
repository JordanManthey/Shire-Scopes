// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MySQLDBConnectionActor.h"
#include "MatchPlayerData.h"
#include "DatabaseManager.generated.h"

/**
 * 
 */
UCLASS()
class SHIRESCOPES_API ADatabaseManager : public AMySQLDBConnectionActor
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ConnectToDatabase();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void UpdateLeaderboard(FMatchPlayerData WinningPlayerData, FMatchPlayerData LosingPlayerData);

protected:

private:
	
};
