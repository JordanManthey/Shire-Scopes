// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomPlayerController.h"
#include "LobbyPlayerController.generated.h"

class ALobbyGameState;

/**
 * The player controller to be used for LobbyGameMode instances. 
 * Used for interacting with Lobby UI. Refer to BP_LobbyPlayerController for more functionality.
 */

UCLASS()
class SHIRESCOPES_API ALobbyPlayerController : public ACustomPlayerController
{
	GENERATED_BODY()

public:

protected:

	UPROPERTY(BLueprintReadOnly)
	ALobbyGameState* LobbyGameState;

	virtual void BeginPlay() override;

private:

};
