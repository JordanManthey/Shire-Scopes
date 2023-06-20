// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyGameMode.h"
#include "LobbyGameState.h"
#include "ShirePlayerState.h"
#include "ShireGameInstance.h"
#include "LobbyPlayerController.h"
#include "Kismet/GameplayStatics.h"

ALobbyGameMode::ALobbyGameMode()
{

}

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer) 
{
	Super::PostLogin(NewPlayer);
}

void ALobbyGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	if (CanPlayerJoin(NewPlayer))
	{
		ALobbyGameState* lobbyGameState = Cast<ALobbyGameState>(GameState);
		lobbyGameState->AddLobbyPlayerData(NewPlayer);
		OnPlayerJoinOrLeave.Broadcast(lobbyGameState->GetAllPlayersReady());
	}
	else 
	{
		Cast<ACustomPlayerController>(NewPlayer)->Client_TravelToLevel(TEXT("MainMenu"), TEXT("Returning To Menu"), TEXT("gameMessage=full"));
	}
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	APlayerController* ExitingPlayer = Cast<APlayerController>(Exiting);
	ALobbyGameState* lobbyGameState = Cast<ALobbyGameState>(GameState);
	lobbyGameState->OnPlayerLogout(ExitingPlayer);
	OnPlayerJoinOrLeave.Broadcast(lobbyGameState->GetAllPlayersReady());
}

void ALobbyGameMode::HandleStartingNewBot()
{
	ALobbyGameState* lobbyGameState = Cast<ALobbyGameState>(GameState);
	lobbyGameState->AddLobbyBotData();
	OnPlayerJoinOrLeave.Broadcast(lobbyGameState->GetAllPlayersReady());
}

void ALobbyGameMode::RemoveBot(int32 BotID)
{
	ALobbyGameState* lobbyGameState = Cast<ALobbyGameState>(GameState);
	lobbyGameState->RemoveLobbyBotData(BotID);
	OnPlayerJoinOrLeave.Broadcast(lobbyGameState->GetAllPlayersReady());
}

void ALobbyGameMode::SwapPlayerControllers(APlayerController* OldPC, APlayerController* NewPC)
{
	Super::SwapPlayerControllers(OldPC, NewPC);
}

void ALobbyGameMode::StartGame(const FString& LevelName, const FString& GameModeName, bool IsMatchmaking)
{
	ALobbyGameState* lobbyGameState = Cast<ALobbyGameState>(GameState);
	lobbyGameState->CacheTeamDataInPlayerStates(); // For Seamless Travel.
	PersistLobbyPlayersData(); // For Normal Travel.

	int playerCount = 0;
	for (int i = 0; i < lobbyGameState->LobbyPlayersData.Num(); i++)
	{
		FLobbyPlayerData lobbyPlayerData = lobbyGameState->LobbyPlayersData[i];
		if ( !ACustomGameState::IsBotID(lobbyPlayerData.PlayerID) )
		{
			playerCount++;
		}
	}

	FString options = FString(TEXT("?playercount") + FString::FromInt(playerCount)) + FString(TEXT("?matchmaking=") + FString::FromInt(int(IsMatchmaking)));

	if ( GameModeName.Len() > 0 )
	{
		// Format: /.../.../BP_SomeGameMode.BP_SomeGameMode_C
		options = options + TEXT("?Game=/Game/Blueprints/Game/") + GameModeName.LeftChop(2) + TEXT(".") + GameModeName;
	}

	TravelToLevel(LevelName, true, TEXT("Loading Map"), options);
}

void ALobbyGameMode::PersistLobbyPlayersData()
{
	ALobbyGameState* lobbyGameState = Cast<ALobbyGameState>(GameState);
	Cast<UShireGameInstance>(GetGameInstance())->LobbyPlayersData = lobbyGameState->LobbyPlayersData;
}
