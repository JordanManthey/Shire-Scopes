// Fill out your copyright notice in the Description page of Project Settings.

#include "CustomGameMode.h"
#include "ShireGameInstance.h"
#include "CustomPlayerController.h"
#include "CustomGameState.h"
#include "Kismet/GameplayStatics.h"

ACustomGameMode::ACustomGameMode()
{

}

void ACustomGameMode::BeginPlay()
{
	Super::BeginPlay();
	CheckPreventSeamlessTravel(); // DEBUG

	ACustomGameState* customGameState = Cast<ACustomGameState>(GameState);
	if (customGameState)
	{
		customGameState->OnPlayerArrayUpdate.AddDynamic(this, &ACustomGameMode::OnPlayerArrayUpdate);
	}

	SetMaxPlayers();

}

void ACustomGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
}

void ACustomGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
}

void ACustomGameMode::OnPlayerArrayUpdate()
{
	// Check if Server is pending travel and all clients have left.
	if (PendingTravelLevelName.Len() > 0 && GameState->PlayerArray.Num() == 1)
	{
		ACustomPlayerController* serverPlayer = Cast<ACustomPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		serverPlayer->Client_TravelToLevel(PendingTravelLevelName);
	}
}

void ACustomGameMode::CheckPreventSeamlessTravel()
{
	UShireGameInstance* shireGameInstance = Cast<UShireGameInstance>(GetGameInstance());
	if (shireGameInstance->Debug_bPreventSeamlessTravel)
	{
		bUseSeamlessTravel = false;
	}
}

void ACustomGameMode::FlagSeamlessTravelWidgets() const
{
	ACustomGameState* customGameState = Cast<ACustomGameState>(GameState);
	customGameState->Multicast_FlagSeamlessTravelWidgets();
}

void ACustomGameMode::CacheGameStateClass() const
{
	ACustomGameState* customGameState = Cast<ACustomGameState>(GameState);
	customGameState->Multicast_CacheGameStateClass();
}

void ACustomGameMode::ToggleLoadingScreens(bool bIsVisible, const FString& LoadingMessage) const
{
	TArray<APlayerState*> playerStates = GameState->PlayerArray;
	for (int i = 0; i < playerStates.Num(); i++)
	{
		ACustomPlayerController* player = Cast<ACustomPlayerController>(playerStates[i]->GetPlayerController());
		player->Client_ToggleLoadingScreen(bIsVisible, LoadingMessage);
	}
}

void ACustomGameMode::TravelToLevel(const FString& LevelName, bool bServerTravel, const FString& LoadingMessage, const FString& Options)
{
	if (bServerTravel)
	{
		CacheGameStateClass();
		FlagSeamlessTravelWidgets();
		ToggleLoadingScreens(true, LoadingMessage);
		ACustomPlayerController* serverPlayer = Cast<ACustomPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		FString url = FString(TEXT("/Game/Levels/")) + LevelName + TEXT("?listen?travelType=server") + Options;
		GetWorld()->ServerTravel(url, true);
	}
	else
	{
		PendingTravelLevelName = LevelName;
		APlayerState* serverPlayerState = UGameplayStatics::GetPlayerState(GetWorld(), 0);
		TArray<APlayerState*> playerStates = GameState->PlayerArray;

		if (playerStates.Num() == 1) // Only Server is in this session. Don't need to wait for client logouts.
		{
			ACustomPlayerController* serverPlayer = Cast<ACustomPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
			serverPlayer->Client_TravelToLevel(PendingTravelLevelName);
		}
		else
		{
			for (int i = 0; i < playerStates.Num(); i++)
			{
				if (playerStates[i]->GetPlayerId() == serverPlayerState->GetPlayerId())
				{
					continue;
				}

				ACustomPlayerController* player = Cast<ACustomPlayerController>(playerStates[i]->GetPlayerController());
				player->Client_TravelToLevel(LevelName, LoadingMessage, Options);
			}
		}
	}
}

bool ACustomGameMode::CanPlayerJoin(APlayerController* NewPlayer)
{
	SetMaxPlayers();
	int currentPlayerCount = GameState->PlayerArray.Num();

	if (currentPlayerCount > MaxPlayers)
	{
		return false;
	}

	return true;
}

void ACustomGameMode::SetMaxPlayers()
{
	MaxPlayers = Cast<UShireGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()))->SessionMaxPlayers;
}

void ACustomGameMode::OnLocalControllerPossessAndBeginPlay()
{
	LocalControllerReadyCount++;

	SetMaxPlayers();
	if (LocalControllerReadyCount == MaxPlayers)
	{
		OnAllLocalControllersReady();
	}
}

void ACustomGameMode::OnAllLocalControllersReady()
{
	SetupData.bAllLocalControllersReady_Complete = true;
}
