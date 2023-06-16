// Fill out your copyright notice in the Description page of Project Settings.

#include "ShireGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"

FLobbyPlayerData* UShireGameInstance::GetLobbyPlayerData(FString PlayerName)
{
	for (int i = 0; i < LobbyPlayersData.Num(); i++)
	{
		FLobbyPlayerData lobbyPlayerData = LobbyPlayersData[i];
		if (lobbyPlayerData.PlayerName == PlayerName)
		{
			return &LobbyPlayersData[i];
		}
	}

	return nullptr;

}

FLobbyPlayerData* UShireGameInstance::GetLobbyBotData(int32 BotID)
{
	for (int i = 0; i < LobbyPlayersData.Num(); i++)
	{
		FLobbyPlayerData lobbyPlayerData = LobbyPlayersData[i];
		if (lobbyPlayerData.PlayerID == BotID)
		{
			return &LobbyPlayersData[i];
		}
	}

	return nullptr;

}

void UShireGameInstance::CacheGameStateClass()
{
	AGameStateBase* gameState = UGameplayStatics::GetGameState(GetWorld());
	PreviousGameStateClass = gameState->GetClass();

}

void UShireGameInstance::EndClientSession_Implementation(APlayerController* Player)
{
	// Defined in BP.
}

TArray<FLobbyPlayerData> UShireGameInstance::GetAllLobbyPlayersData()
{
	TArray<FLobbyPlayerData> allLobbyPlayers;
	for (int i = 0; i < LobbyPlayersData.Num(); i++)
	{
		FLobbyPlayerData lobbyPlayerData = LobbyPlayersData[i];
		if ( !ACustomGameState::IsBotID(lobbyPlayerData.PlayerID) )
		{
			allLobbyPlayers.Add(lobbyPlayerData);
		}
	}

	return allLobbyPlayers;
}

TArray<FLobbyPlayerData> UShireGameInstance::GetAllLobbyBotsData()
{
	TArray<FLobbyPlayerData> allBotPlayers;
	for (int i = 0; i < LobbyPlayersData.Num(); i++)
	{
		FLobbyPlayerData lobbyPlayerData = LobbyPlayersData[i];
		if ( ACustomGameState::IsBotID(lobbyPlayerData.PlayerID) )
		{
			allBotPlayers.Add(lobbyPlayerData);
		}
	}

	return allBotPlayers;
}
