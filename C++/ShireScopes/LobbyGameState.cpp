// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyGameState.h"
#include "Net/UnrealNetwork.h"
#include "ShirePlayerState.h"
#include "GameFramework/PlayerState.h"

void ALobbyGameState::BeginPlay()
{
	Super::BeginPlay();
}

void ALobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyGameState, MapIndex);
	DOREPLIFETIME(ALobbyGameState, ModeIndex);
	DOREPLIFETIME(ALobbyGameState, BotCount);
	DOREPLIFETIME(ALobbyGameState, LobbyPlayersData);
}

void ALobbyGameState::OnRep_LobbyUpdate()
{
	OnLobbyUpdate.Broadcast();
}

void ALobbyGameState::AddLobbyPlayerData(APlayerController* NewPlayer)
{
	APlayerState* newPlayerState = NewPlayer->PlayerState;
	FLobbyPlayerData newLobbyPlayerData = FLobbyPlayerData();

	newLobbyPlayerData.PlayerName = newPlayerState->GetPlayerName();
	newLobbyPlayerData.PlayerID = newPlayerState->GetPlayerId();

	if (NewPlayer->HasAuthority() && NewPlayer->IsLocalController())
	{
		newLobbyPlayerData.ReadyStatus = true;
	}

	LobbyPlayersData.Add(newLobbyPlayerData);
}

void ALobbyGameState::RemoveLobbyPlayerData(APlayerController* Player)
{
	for (int i = 0; i < LobbyPlayersData.Num(); i++)
	{
		FLobbyPlayerData lobbyPlayerData = LobbyPlayersData[i];
		if (Player->PlayerState->GetPlayerId() == lobbyPlayerData.PlayerID)
		{
			LobbyPlayersData.RemoveAt(i);
			break;
		}
	}
}

void ALobbyGameState::AddLobbyBotData()
{
	BotCount++;
	FLobbyPlayerData newLobbyPlayerData = FLobbyPlayerData();

	newLobbyPlayerData.PlayerName = FString(TEXT("Bot "));
	newLobbyPlayerData.PlayerName.AppendInt(BotCount);
	newLobbyPlayerData.PlayerID = GenerateBotID();
	newLobbyPlayerData.ReadyStatus = true;

	LobbyPlayersData.Add(newLobbyPlayerData);
}

void ALobbyGameState::RemoveLobbyBotData(int32 BotID)
{
	for (int i = 0; i < LobbyPlayersData.Num(); i++)
	{
		FLobbyPlayerData lobbyPlayerData = LobbyPlayersData[i];
		if (BotID == lobbyPlayerData.PlayerID)
		{
			LobbyPlayersData.RemoveAt(i);
			break;
		}
	}

	BotCount--;
}

int ALobbyGameState::GenerateBotID() const
{
	return 1000 + BotCount;
}

void ALobbyGameState::OnPlayerLogout(APlayerController* Player)
{
	RemoveLobbyPlayerData(Player);
}

FLobbyPlayerData* ALobbyGameState::GetLobbyPlayerData(int32 PlayerID)
{
	for (int i = 0; i < LobbyPlayersData.Num(); i++)
	{
		FLobbyPlayerData lobbyPlayerData = LobbyPlayersData[i];
		if (lobbyPlayerData.PlayerID == PlayerID)
		{
			return &LobbyPlayersData[i];
		}
	}

	return nullptr;
}

void ALobbyGameState::CacheTeamDataInPlayerStates()
{
	for (int i = 0; i < PlayerArray.Num(); i++)
	{
		AShirePlayerState* playerState = Cast<AShirePlayerState>(PlayerArray[i]);
		playerState->Team = GetLobbyPlayerData(playerState->GetPlayerId())->Team;
	}
}

bool ALobbyGameState::GetAllPlayersReady() const
{
	bool bAllPlayersReady = true;

	for (int i = 0; i < LobbyPlayersData.Num(); i++)
	{
		FLobbyPlayerData lobbyPlayerData = LobbyPlayersData[i];
		if (!lobbyPlayerData.ReadyStatus)
		{
			bAllPlayersReady = false;
		}
	}

	return bAllPlayersReady;

}

void ALobbyGameState::UpdateLobbyPlayerTeam(int32 PlayerID, int NewTeam)
{
	FLobbyPlayerData* lobbyPlayerData = GetLobbyPlayerData(PlayerID);
	lobbyPlayerData->Team = NewTeam;
}

void ALobbyGameState::UpdateLobbyPlayerReadyStatus(int32 PlayerID, bool NewReadyStatus)
{
	FLobbyPlayerData* lobbyPlayerData = GetLobbyPlayerData(PlayerID);
	lobbyPlayerData->ReadyStatus = NewReadyStatus;
	OnReadyStatusUpdate.Broadcast(GetAllPlayersReady());
}
