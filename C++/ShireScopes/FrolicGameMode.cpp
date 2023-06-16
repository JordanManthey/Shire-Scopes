// Fill out your copyright notice in the Description page of Project Settings.


#include "FrolicGameMode.h"
#include "ShireGameState.h"
#include "ShirePlayerController.h"
#include "ShireAIController.h"

void AFrolicGameMode::OnAllPlayersLogin()
{
	Super::OnAllPlayersLogin();
	bInitialSpawnsComplete = true;
}

void AFrolicGameMode::SpawnPlayer(AController* Player)
{
	Super::SpawnPlayer(Player);

	if ( bInitialSpawnsComplete )
	{
		AShireAIController* botController = Cast<AShireAIController>(Player);
		if (botController)
		{
			botController->StartBehavior();
		}

		AShirePlayerController* playerController = Cast<AShirePlayerController>(Player);
		if (playerController)
		{
			playerController->GetPawn()->EnableInput(playerController);
		}
	}

}
