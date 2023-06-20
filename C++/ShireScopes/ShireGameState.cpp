// Fill out your copyright notice in the Description page of Project Settings.

#include "ShireGameState.h"
#include "ShireCharacter.h"
#include "ShirePlayerState.h"
#include "ShireGameInstance.h"
#include "ShireAIController.h"
#include "ShirePlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "steam/steam_api.h"

AShireGameState::AShireGameState()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AShireGameState::BeginPlay()
{
	Super::BeginPlay();
}

void AShireGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
}

void AShireGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShireGameState, MatchPlayersData);
}

void AShireGameState::OnRep_ReplicatedHasBegunPlay() // Only triggers on Clients, not Server.
{
	Super::OnRep_ReplicatedHasBegunPlay();
}

void AShireGameState::OnPlayerLogout(APlayerController* Player)
{
	RemoveMatchPlayerData(Player);
	OnPlayerDataUpdate.Broadcast();
}

void AShireGameState::RemoveMatchPlayerData(APlayerController* Player)
{
	for (int i = 0; i < MatchPlayersData.Num(); i++)
	{
		FMatchPlayerData matchPlayerData = MatchPlayersData[i];
		if (Player->PlayerState->GetPlayerId() == matchPlayerData.PlayerID)
		{
			MatchPlayersData.RemoveAt(i);
			break;
		}
	}
}

void AShireGameState::OnRep_PlayerDataUpdate()
{
	OnPlayerDataUpdate.Broadcast();
}

void AShireGameState::AddMatchPlayerData(AController* NewPlayer)
{
	UShireGameInstance* shireGameInstance = Cast<UShireGameInstance>(GetGameInstance());
	FMatchPlayerData newMatchPlayerData = FMatchPlayerData();
	AShirePlayerState* newPlayerState = Cast<AShirePlayerState>(NewPlayer->PlayerState);
	AShireAIController* newBotController = Cast<AShireAIController>(NewPlayer);
	FLobbyPlayerData* persistedPlayerData = nullptr;

	if ( newPlayerState ) // If not a bot
	{
		newMatchPlayerData.PlayerName = newPlayerState->GetPlayerName();
		newMatchPlayerData.PlayerID = newPlayerState->GetPlayerId();
		newMatchPlayerData.Team = newPlayerState->Team; // For Seamless Travel.
		persistedPlayerData = shireGameInstance->GetLobbyPlayerData(newMatchPlayerData.PlayerName); // Lookup by PlayerName since PlayerID can change after Normal Travel.
	}
	else if ( newBotController )
	{
		
		newMatchPlayerData.PlayerName = newBotController->BotName;
		newMatchPlayerData.PlayerID = newBotController->BotID;
		newMatchPlayerData.SteamID = FString::FromInt(newBotController->BotID); // Giving bots a SteamID for testing purposes.
		newMatchPlayerData.Team = newBotController->Team;
		persistedPlayerData = shireGameInstance->GetLobbyBotData(newMatchPlayerData.PlayerID);
	}

	// DEBUG
	int targetTeam = 1;
	UShireGameInstance* gameInstance = Cast<UShireGameInstance>(GetGameInstance());
	if ( !persistedPlayerData )
	{
		if (gameInstance->Debug_PreviousTeamAssignment == 1) { targetTeam = 2; }
		gameInstance->Debug_PreviousTeamAssignment = targetTeam;
	}
	else if ( persistedPlayerData )
	{
		targetTeam = persistedPlayerData->Team;
	}

	if ( newPlayerState )
	{
		newPlayerState->Team = targetTeam;
	}

	// NOTE: debug bot team assignment is handled in AShireGameMode::SetupAllBots

	newMatchPlayerData.Team = targetTeam;
	MatchPlayersData.Add(newMatchPlayerData);
	OnPlayerDataUpdate.Broadcast();

	// Below must come after adding to MatchPlayersData array so that the SteamID update can occur.
	if (newPlayerState) // If not a bot
	{
		// Ask the controlling client of NewPlayer to assign their own SteamID to the server. 
		// If not connected to Steam, make it the PlayerID for testing purposes.
		if (SteamAPI_Init())
		{
			AShirePlayerController* shirePlayerController = Cast<AShirePlayerController>(NewPlayer);
			shirePlayerController->Client_UpdateMatchPlayerSteamID(newMatchPlayerData.PlayerID);
		}
		else
		{
			newMatchPlayerData.SteamID = FString::FromInt(newMatchPlayerData.PlayerID);
		}
	}
}

FMatchPlayerData* AShireGameState::GetMatchPlayerData(int32 PlayerID)
{
	for (int i = 0; i < MatchPlayersData.Num(); i++)
	{
		FMatchPlayerData lobbyPlayerData = MatchPlayersData[i];
		if (lobbyPlayerData.PlayerID == PlayerID)
		{
			return &MatchPlayersData[i];
		}
	}

	return nullptr;
}

void AShireGameState::UpdateMatchPlayerHealing(int32 PlayerID, int DeltaHealing)
{
	FMatchPlayerData* MatchPlayerData = GetMatchPlayerData(PlayerID);
	MatchPlayerData->HealingCount += DeltaHealing;
	OnPlayerDataUpdate.Broadcast();
}

void AShireGameState::UpdateMatchPlayerDamage(int32 PlayerID, int DeltaDamage)
{
	FMatchPlayerData* MatchPlayerData = GetMatchPlayerData(PlayerID);
	MatchPlayerData->DamageCount += DeltaDamage;
	OnPlayerDataUpdate.Broadcast();
}

void AShireGameState::UpdateMatchPlayerKills(int32 PlayerID, int DeltaKills)
{
	FMatchPlayerData* MatchPlayerData = GetMatchPlayerData(PlayerID);
	MatchPlayerData->KillCount += DeltaKills;
	OnPlayerDataUpdate.Broadcast();
}

void AShireGameState::UpdateMatchPlayerDeaths(int32 PlayerID, int DeltaDeaths)
{
	FMatchPlayerData* MatchPlayerData = GetMatchPlayerData(PlayerID);
	MatchPlayerData->DeathCount += DeltaDeaths;
	OnPlayerDataUpdate.Broadcast();
}

void AShireGameState::Server_UpdateMatchPlayerSteamID_Implementation(int32 PlayerID, const FString& SteamID)
{
	FMatchPlayerData* matchPlayerData = GetMatchPlayerData(PlayerID);
	if ( matchPlayerData )
	{
		matchPlayerData->SteamID = SteamID;
	}
}

void AShireGameState::OnPlayerTakeDamage(AController* DamageReceiver, AController* DamageDealer, float DamageAmount)
{
	int32 DamageDealerID;
	if (DamageDealer->PlayerState)
	{
		DamageDealerID = DamageDealer->PlayerState->GetPlayerId();
	}
	else
	{
		DamageDealerID = Cast<AShireAIController>(DamageDealer)->BotID;
	}

	UpdateMatchPlayerDamage(DamageDealerID, DamageAmount);
	
	// Update Kills/Deaths if DamageReceiver Character died from the damage. 

	if (DamageReceiver)
	{
		int test = 500;
		if (DamageReceiver->GetPawn())
		{
			test = 1000;
			AShireCharacter* damageReceiverCharacter2 = Cast<AShireCharacter>(DamageReceiver->GetPawn());
		}
	}

	AShireCharacter* damageReceiverCharacter = Cast<AShireCharacter>(DamageReceiver->GetPawn());
	if ( damageReceiverCharacter && damageReceiverCharacter->IsDead() )
	{	
		int32 DamageReceiverID;
		if (DamageReceiver->PlayerState)
		{
			DamageReceiverID = DamageReceiver->PlayerState->GetPlayerId();
		}
		else
		{
			DamageReceiverID = Cast<AShireAIController>(DamageReceiver)->BotID;
		}

		UpdateMatchPlayerDeaths(DamageReceiverID, 1);
		UpdateMatchPlayerKills(DamageDealerID, 1);
	}
}

void AShireGameState::OnPlayerTakeHealing(AController* HealingReceiver, AController* HealingDealer, float HealingAmount)
{
	int32 HealingDealerID;
	if (HealingDealer->PlayerState)
	{
		HealingDealerID = HealingDealer->PlayerState->GetPlayerId();
	}
	else
	{
		HealingDealerID = Cast<AShireAIController>(HealingDealer)->BotID;
	}

	UpdateMatchPlayerHealing(HealingDealerID, HealingAmount);
}

void AShireGameState::Multicast_StartRoundCountdown_Implementation()
{
	OnRoundCountdown.Broadcast();
	FTimerDelegate timerDelegate;
	FTimerHandle unusedHandle;
	timerDelegate.BindUFunction(this, FName("StartRound"));
	GetWorldTimerManager().SetTimer(unusedHandle, timerDelegate, StartRoundDelay, false);
}

void AShireGameState::BindToCharacterEvents(AShireCharacter* ShireCharacter)
{
	ShireCharacter->OnTakeDamage.AddDynamic(this, &AShireGameState::OnPlayerTakeDamage);
	ShireCharacter->OnTakeHealing.AddDynamic(this, &AShireGameState::OnPlayerTakeHealing);
}

void AShireGameState::StartRound()
{
	OnRoundStart.Broadcast();
}

void AShireGameState::SetLocalPlayer(APlayerController* TargetPlayer)
{
	LocalPlayer = TargetPlayer;
	OnLocalPlayerSet.Broadcast(LocalPlayer);
}

void AShireGameState::Multicast_OnAllLocalCharactersReady_Implementation()
{
	OnAllLocalCharactersReady.Broadcast();
}

void AShireGameState::Multicast_OnRoundOver_Implementation(int WinningTeam)
{
	OnRoundOver.Broadcast(WinningTeam);
}

void AShireGameState::Multicast_OnMatchOver_Implementation(int WinningTeam)
{
	OnMatchOver.Broadcast(WinningTeam);
}
