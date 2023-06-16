// Fill out your copyright notice in the Description page of Project Settings.

#include "EliminationGameMode.h"
#include "ShireGameInstance.h"
#include "ShireGameState.h"
#include "ShireCharacter.h"
#include "ShirePlayerState.h"
#include "ShireAIController.h"
#include "Engine/PlayerStartPIE.h"
#include "EngineUtils.h"

void AEliminationGameMode::OnPlayerDeath(AShireCharacter* DeathCharacter)
{
	if (AnyTeamHasWonRound())
	{
		EndRound();
	}
}

int AEliminationGameMode::GetRoundWinningTeam() const
{
	for (auto& Elem : MatchTeamsData)
	{
		if (!IsTeamDead(Elem.Key))
		{
			return Elem.Key;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("WARNING: Could not determine round winner."));
	return 1;
}

bool AEliminationGameMode::AnyTeamHasWonRound() const
{
	int teamsAlive = 0;
	for (auto& Elem : MatchTeamsData)
	{
		if (!IsTeamDead(Elem.Key))
		{
			teamsAlive++;
		}
	}
	
	return (teamsAlive == 1);
}
