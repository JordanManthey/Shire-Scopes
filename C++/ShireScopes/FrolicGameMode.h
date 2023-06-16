// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShireGameMode.h"
#include "FrolicGameMode.generated.h"

/**
 * 
 */
UCLASS()
class SHIRESCOPES_API AFrolicGameMode : public AShireGameMode
{
	GENERATED_BODY()

public:

protected:

	bool bInitialSpawnsComplete { false };

	virtual void OnAllPlayersLogin() override;
	virtual void SpawnPlayer(AController* Player);

private:
};
