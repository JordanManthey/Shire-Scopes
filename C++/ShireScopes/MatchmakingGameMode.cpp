// Fill out your copyright notice in the Description page of Project Settings.

#include "MatchmakingGameMode.h"
#include "LobbyGameState.h"

FString AMatchmakingGameMode::GetRandomLevelName_Implementation() const
{
	// Defined in BP
	return FString();
}

void AMatchmakingGameMode::OnStartGame_Implementation()
{
	// Defined in BP;
}

void AMatchmakingGameMode::SetMaxPlayers()
{
	MaxPlayers = 2;
}

void AMatchmakingGameMode::OnAllLocalControllersReady()
{
	ALobbyGameState* lobbyGameState = Cast<ALobbyGameState>(GameState);
	lobbyGameState->LobbyPlayersData[0].Team = 2;
	OnStartGame();
}
