// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShireGameMode.h"
#include "ShireCharacter.h"
#include "ShireSpectator.h"
#include "ShireGameState.h"
#include "ShirePlayerState.h"
#include "ShirePlayerController.h"
#include "ShireAIController.h"
#include "ShireGameInstance.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/PlayerStartPIE.h"
#include "EngineUtils.h"

AShireGameMode::AShireGameMode()
{
	// Set default pawn class to our Blueprinted character.
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Blueprints/Actors/BP_ShireCharacter"));
	ShirePawnClass = PlayerPawnClassFinder.Class;
	//DefaultPawnClass = PlayerPawnClassFinder.Class;
}

void AShireGameMode::BeginPlay()
{
	Super::BeginPlay();
	UShireGameInstance* shireGameInstance = Cast<UShireGameInstance>(GetGameInstance());
	shireGameInstance->SetupDatabaseManager();
}

void AShireGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	if (CanPlayerJoin(NewPlayer))
	{
		// Bind to Character events.
		AShireCharacter* character = Cast<AShireCharacter>(NewPlayer->GetPawn());
		if (character)
		{
			character->OnDeath.AddDynamic(this, &AShireGameMode::OnPlayerDeath);
		}

		// Add Player Data
		AShireGameState* shireGameState = Cast<AShireGameState>(GameState);
		shireGameState->AddMatchPlayerData(NewPlayer);

		UShireGameInstance* shireGameInstance = Cast<UShireGameInstance>(GetGameInstance());
		FLobbyPlayerData* persistedPlayerData = shireGameInstance->GetLobbyPlayerData(NewPlayer->PlayerState->GetPlayerName());
		int targetPlayerCount = shireGameInstance->GetAllLobbyPlayersData().Num();
		if (persistedPlayerData)
		{
			MatchTeamsData.Add(persistedPlayerData->Team, FMatchTeamData());
		}
		else // DEBUG
		{
			targetPlayerCount = shireGameInstance->Debug_TargetPlayerCount;
			MatchTeamsData.Add(1, FMatchTeamData());
			MatchTeamsData.Add(2, FMatchTeamData());
		}

		if (++PlayerLoginCount == targetPlayerCount)
		{
			OnAllPlayersLogin();
		}
	}
	else
	{
		Cast<AShirePlayerController>(NewPlayer)->Client_TravelToLevel(TEXT("MainMenu"), TEXT("Returning To Menu"), TEXT("gameMessage=full"));
	}

}

void AShireGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
}

void AShireGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	APlayerController* ExitingPlayer = Cast<APlayerController>(Exiting);
	AShireGameState* shireGameState = Cast<AShireGameState>(GameState);
	shireGameState->OnPlayerLogout(ExitingPlayer);
}

void AShireGameMode::StartRound()
{
	ResetSpawnPoints();
	SpawnAllPlayers();
	SpawnAllBots();
	AShireGameState* shireGameState = Cast<AShireGameState>(GameState);
	shireGameState->Multicast_StartRoundCountdown();
	CurrentRound++;
}

void AShireGameMode::EndRound()
{
	int winningTeam = GetRoundWinningTeam();
	MatchTeamsData[winningTeam].RoundsWon++;

	if (AnyTeamHasWonMatch())
	{
		EndMatch();
		return;
	}

	AShireGameState* shireGameState = Cast<AShireGameState>(GameState);
	shireGameState->Multicast_OnRoundOver(winningTeam);

	FTimerDelegate timerDelegate;
	FTimerHandle unusedHandle;
	timerDelegate.BindUFunction(this, FName(TEXT("StartRound")));
	GetWorldTimerManager().SetTimer(unusedHandle, timerDelegate, shireGameState->RoundTransitionDelay, false);
}

void AShireGameMode::EndMatch()
{
	AShireGameState* shireGameState = Cast<AShireGameState>(GameState);
	shireGameState->Multicast_OnMatchOver(GetMatchWinningTeam());

	// Send match results to database if this was a matchmaking session.
	FString isMatchmaking = UGameplayStatics::ParseOption(OptionsString, TEXT("matchmaking"));
	if ( isMatchmaking == "1" )
	{
		UpdateLeaderboard();
	}

	FTimerDelegate timerDelegate;
	FTimerHandle unusedHandle;
	timerDelegate.BindUFunction(this, FName("ExitMatch"));
	GetWorldTimerManager().SetTimer(unusedHandle, timerDelegate, shireGameState->ExitMatchTransitionDelay, false);
}

void AShireGameMode::ExitMatch()
{
	TravelToLevel(TEXT("Lobby"), true, TEXT("Returning To Lobby"));
}

int AShireGameMode::GetRoundWinningTeam() const
{
	// Define in child class.
	UE_LOG(LogTemp, Warning, TEXT("WARNING: AShireGameMode::GetRoundWinningTeam() called. This should be Abstract."));
	return 1;
}

bool AShireGameMode::AnyTeamHasWonRound() const
{
	// Define in child class.
	return false;
}

int AShireGameMode::GetMatchWinningTeam() const
{
	int maxRoundsWon = 0;
	int maxWinningTeam = 1;

	for (auto& Elem : MatchTeamsData)
	{
		if (Elem.Value.RoundsWon > maxRoundsWon)
		{
			maxRoundsWon = Elem.Value.RoundsWon;
			maxWinningTeam = Elem.Key;
		}
	}

	return maxWinningTeam;
}

bool AShireGameMode::AnyTeamHasWonMatch() const
{
	int winningTeam = GetMatchWinningTeam();
	int roundWinsNeeded = (MaxRound / 2) + 1;

	if (MatchTeamsData[winningTeam].RoundsWon >= roundWinsNeeded)
	{
		return true;
	}

	return false;
}

bool AShireGameMode::IsTeamDead(int TargetTeam) const
{
	int teamSize = 0;
	int deathCount = 0;
	AShireGameState* shireGameState = Cast<AShireGameState>(GameState);
	for (int i = 0; i < GameState->PlayerArray.Num(); i++)
	{
		AShirePlayerState* playerState = Cast<AShirePlayerState>(GameState->PlayerArray[i]);
		AShireCharacter* character = Cast<AShireCharacter>(playerState->GetPawn());
		AShireSpectator* spectator = Cast<AShireSpectator>(playerState->GetPawn());

		if ( playerState->Team == TargetTeam )
		{
			teamSize++;

			// Consider dead if character is dead or if the player is spectating.
			if ( spectator || ( character && character->IsDead() ) )
			{
				deathCount++;
			}
		}
	}

	for (int i = 0; i < BotControllers.Num(); i++)
	{
		AShireAIController* botController = BotControllers[i];
		AShireCharacter* botCharacter = Cast<AShireCharacter>(botController->GetPawn());
		if (botCharacter->GetTeam() == TargetTeam)
		{
			teamSize++;
			if (botCharacter->IsDead())
			{
				deathCount++;
			}
		}
	}

	return (deathCount == teamSize);
}

void AShireGameMode::UpdateLeaderboard()
{
	UShireGameInstance* shireGameInstance = Cast<UShireGameInstance>(GetGameInstance());
	ADatabaseManager* databaseManager = shireGameInstance->DatabaseManager;

	if ( databaseManager )
	{
		int winningTeam = GetMatchWinningTeam();
		FMatchPlayerData winningPlayerData;
		FMatchPlayerData losingPlayerData;

		AShireGameState* shireGameState = Cast<AShireGameState>(GameState);
		for (int i = 0; i < shireGameState->MatchPlayersData.Num(); i++)
		{
			FMatchPlayerData matchPlayerData = shireGameState->MatchPlayersData[i];
			if (matchPlayerData.Team == winningTeam)
			{
				winningPlayerData = matchPlayerData;
			}
			else
			{
				losingPlayerData = matchPlayerData;
			}
		}

		databaseManager->UpdateLeaderboard(winningPlayerData, losingPlayerData);
	}
}

void AShireGameMode::ResetSpawnPoints()
{
	TeamToSpawnMap.Empty();
	OccupiedSpawnPoints.Empty();
	UnoccupiedSpawnPoints.Empty();

	APlayerStart* FoundPlayerSpawn = nullptr;
	UWorld* World = GetWorld();
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		APlayerStart* PlayerStart = *It;
		UnoccupiedSpawnPoints.Add(PlayerStart);
	}
}

void AShireGameMode::OnLocalCharacterPossessAndBeginPlay()
{
	LocalCharacterReadyCount++;
	UShireGameInstance* shireGameInstance = Cast<UShireGameInstance>(GetGameInstance());

	// DEBUG
	int targetLocalCount = MaxPlayers;
	if ( shireGameInstance->Debug_TargetPlayerCount > 0 ) { targetLocalCount = shireGameInstance->Debug_TargetPlayerCount; }

	if ( LocalCharacterReadyCount == targetLocalCount )
	{
		OnAllLocalCharactersReady();
	}
}

void AShireGameMode::OnAllLocalControllersReady()
{
	SetupData.bAllLocalControllersReady_Complete = true;

	if (SetupData.bAllLocalCharactersReady_Complete)
	{
		OnAllLocalControllersAndCharactersReady();
	}
}

void AShireGameMode::OnAllLocalCharactersReady()
{
	AShireGameState* shireGameState = Cast<AShireGameState>(GameState);
	shireGameState->Multicast_OnAllLocalCharactersReady();
	shireGameState->Multicast_StartRoundCountdown();
	SetupData.bAllLocalCharactersReady_Complete = true;

	if ( SetupData.bAllLocalControllersReady_Complete )
	{
		OnAllLocalControllersAndCharactersReady();
	}
}

void AShireGameMode::OnAllLocalControllersAndCharactersReady()
{
	ToggleLoadingScreens(false);
}

void AShireGameMode::OnAllPlayersLogin()
{
	DefaultPawnClass = ShirePawnClass;
	SpawnAllPlayers();
	SetupAllBots();
	SpawnAllBots();
}

void AShireGameMode::SetMaxPlayers()
{
	// DEBUGMODE
	UShireGameInstance* shireGameInstance = Cast<UShireGameInstance>(GetGameInstance());
	if ( shireGameInstance->Debug_TargetPlayerCount > 0 )
	{
		MaxPlayers = shireGameInstance->Debug_TargetPlayerCount;
	}
	else
	{
		FString maxPlayersParamValue;
		UKismetSystemLibrary::ParseParamValue(OptionsString, "playercount", maxPlayersParamValue);
		MaxPlayers = FCString::Atoi(*maxPlayersParamValue);
	}
}

void AShireGameMode::OnPlayerDeath(AShireCharacter* DeathCharacter)
{
	QueuePlayerSpawn(Cast<AController>(DeathCharacter->GetController()));
}

void AShireGameMode::QueuePlayerSpawn(AController* Player)
{
	FTimerDelegate timerDelegate;
	FTimerHandle unusedHandle;
	timerDelegate.BindUFunction(this, FName(TEXT("SpawnPlayer")), Player);
	GetWorldTimerManager().SetTimer(unusedHandle, timerDelegate, SpawnDelay, false);
}

void AShireGameMode::SpawnPlayer(AController* Player)
{
	// Spawn player
	AActor* playerStart = ChoosePlayerStart(Player);
	AShireCharacter* prevCharacter = Cast<AShireCharacter>(Player->GetPawn());

	FTransform spawnTransform = FTransform();
	spawnTransform.SetLocation(playerStart->GetActorLocation());
	spawnTransform.SetRotation(playerStart->GetActorRotation().Quaternion());
	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	AShireCharacter* newCharacter = GetWorld()->SpawnActor<AShireCharacter>(GetDefaultPawnClassForController(Player), spawnTransform, spawnParams);
	Player->Possess(newCharacter);
	
	// Possessing the character will override its rotation with the controller rotation. Realign with PlayerStart.
	newCharacter->SetActorRotation(playerStart->GetActorRotation().Quaternion());

	// Destroy previous Character
	if (prevCharacter)
	{
		prevCharacter->Destroy();
	}
	
	// Bind to new Character events
	newCharacter->OnDeath.AddDynamic(this, &AShireGameMode::OnPlayerDeath);
	AShireGameState* shireGameState = Cast<AShireGameState>(GameState);
	shireGameState->BindToCharacterEvents(newCharacter);
	
}

void AShireGameMode::SpawnAllPlayers()
{
	for (int i = 0; i < GameState->PlayerArray.Num(); i++)
	{
		APlayerController* player = GameState->PlayerArray[i]->GetPlayerController();
		SpawnPlayer(player);
	}
}

void AShireGameMode::SetupAllBots()
{
	// DEBUG
	UShireGameInstance* shireGameInstance = Cast<UShireGameInstance>(GetGameInstance());
	if ( shireGameInstance->LobbyPlayersData.Num() == 0 )
	{
		int prevTeam = 2;
		for (int i = 0; i < shireGameInstance->Debug_TargetAICount; i++)
		{
			int targetTeam = 1;
			if ( shireGameInstance->Debug_ForceBotTeamAssignment != 0 )
			{
				targetTeam = shireGameInstance->Debug_ForceBotTeamAssignment;
			} 
			else
			{
				if (prevTeam == 1) { targetTeam = 2; }
				prevTeam = targetTeam;
			}

			AShireAIController* botController = GetWorld()->SpawnActor<AShireAIController>(BotControllerClass, FActorSpawnParameters());
			botController->BotID = 1000 + i;
			botController->Team = targetTeam;
			botController->BotName = FString(TEXT("Bot "));
			botController->BotName.AppendInt(i);

			BotControllers.Add(botController);
			MatchTeamsData.Add(botController->Team, FMatchTeamData());
			AShireGameState* shireGameState = Cast<AShireGameState>(GameState);
			shireGameState->AddMatchPlayerData(botController);
		}

		return;
	}
	
	for (int i = 0; i < shireGameInstance->LobbyPlayersData.Num(); i++)
	{
		FLobbyPlayerData lobbyPlayerData = shireGameInstance->LobbyPlayersData[i];
		if (ACustomGameState::IsBotID(lobbyPlayerData.PlayerID))
		{
			// Create AI Controller
			AShireAIController* botController = GetWorld()->SpawnActor<AShireAIController>(BotControllerClass, FActorSpawnParameters());
			botController->BotID = lobbyPlayerData.PlayerID;
			botController->Team = lobbyPlayerData.Team;
			botController->BotName = lobbyPlayerData.PlayerName;

			BotControllers.Add(botController);

			// Update Teams Data
			MatchTeamsData.Add(botController->Team, FMatchTeamData());

			// Add Player Data
			AShireGameState* shireGameState = Cast<AShireGameState>(GameState);
			shireGameState->AddMatchPlayerData(botController);
		}
	}
}

void AShireGameMode::SpawnAllBots()
{
	for (int i = 0; i < BotControllers.Num(); i++)
	{
		AShireAIController* botController = BotControllers[i];
		SpawnPlayer(botController);
	}
}

AActor* AShireGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	// Check if any teammates have a Spawnpoint.
	// If not Check if any enemies have a Spawnpoint.
	// -- If so Place in the furthest point from that enemy
	// -- If not place at anypoint (you are first placement)
	// If so place to closest open point to teammate

	if (!DefaultPawnClass) { return nullptr; } // Prevent autospawning after loading Level.

	UShireGameInstance* shireGameInstance = Cast<UShireGameInstance>(GetGameInstance());

	int targetTeam = 1;

	FLobbyPlayerData* persistedPlayerData;
	if (Player->PlayerState) // If this is not a Bot.
	{
		AShirePlayerState* shirePlayerState = Cast<AShirePlayerState>(Player->PlayerState);
		targetTeam = shirePlayerState->Team; // In case we are debugging and don't have LobbyPlayerData.
		FString playerName = shirePlayerState->GetPlayerName(); // ISSUE: PlayerName empty at this stage.
		persistedPlayerData = shireGameInstance->GetLobbyPlayerData(playerName); // Lookup by PlayerName since PlayerID can change after Normal Travel.
	}
	else
	{
		AShireAIController* shireAIController = Cast<AShireAIController>(Player);
		targetTeam = shireAIController->Team; // In case we are debugging and don't have LobbyPlayerData.
		int32 botID = shireAIController->BotID;
		persistedPlayerData = shireGameInstance->GetLobbyBotData(botID);
	}

	if (persistedPlayerData)
	{
		targetTeam = persistedPlayerData->Team;
	}

	TArray<int> teamKeys;
	TeamToSpawnMap.GetKeys(teamKeys);
	if (teamKeys.IsEmpty())
	{
		// Player is the first spawn.
		ResetSpawnPoints();
		if (UnoccupiedSpawnPoints.Num() > 0)
		{
			APlayerStart* randomSpawnPoint = UnoccupiedSpawnPoints[FMath::RandRange(0, UnoccupiedSpawnPoints.Num() - 1)];
			UnoccupiedSpawnPoints.Remove(randomSpawnPoint);
			OccupiedSpawnPoints.Add(randomSpawnPoint);

			TArray<APlayerStart*> teamSpawnPoints;
			teamSpawnPoints.Add(randomSpawnPoint);
			TeamToSpawnMap.Add(targetTeam, teamSpawnPoints);
			return randomSpawnPoint;
		}
		else // There must be no PlayerStarts in this level. Create a player start to prevent crash.
		{
			APlayerStart* tempSpawnPoint = GetWorld()->SpawnActor<APlayerStart>();
			tempSpawnPoint->SetActorLocation(FVector(0, 0, 0));
			return tempSpawnPoint;
		}
	}

	if (TeamToSpawnMap.Contains(targetTeam))
	{
		// Player has teammates already spawned.

		// If no unoccupied points, get a occupied team point.
		if (UnoccupiedSpawnPoints.Num() == 0)
		{
			return TeamToSpawnMap[targetTeam][0];
		}

		// Traverse unoccupied points and get closest point from teammate.
		APlayerStart* teammatePoint = TeamToSpawnMap[targetTeam][0];
		float minDistance = 0;
		APlayerStart* closestSpawnPoint;
		for (auto& SpawnPoint : UnoccupiedSpawnPoints)
		{
			float distance = teammatePoint->GetHorizontalDistanceTo(SpawnPoint);
			if (!minDistance || distance < minDistance)
			{
				minDistance = distance;
				closestSpawnPoint = SpawnPoint;
			}
		}

		UnoccupiedSpawnPoints.Remove(closestSpawnPoint);
		OccupiedSpawnPoints.Add(closestSpawnPoint);
		TeamToSpawnMap[targetTeam].Add(closestSpawnPoint);
		return closestSpawnPoint;

	}

	// Player is first Spawn for their team.

	// If no unoccupied points, get a random occupied point.
	if (UnoccupiedSpawnPoints.Num() == 0)
	{
		APlayerStart* randomSpawnPoint = OccupiedSpawnPoints[FMath::RandRange(0, OccupiedSpawnPoints.Num() - 1)];
		return randomSpawnPoint;
	}

	// Traverse SpawnMap keys and see if an enemy team has a spawn.
	for (auto& team : teamKeys)
	{
		if (team != targetTeam)
		{
			// Traverse unoccupied points and get farthest point from enemy.
			APlayerStart* enemyPoint = TeamToSpawnMap[team][0];
			float maxDistance = 0;
			APlayerStart* farthestSpawnPoint;
			for (auto& SpawnPoint : UnoccupiedSpawnPoints)
			{
				float distance = enemyPoint->GetHorizontalDistanceTo(SpawnPoint);
				if (!maxDistance || distance > maxDistance)
				{
					maxDistance = distance;
					farthestSpawnPoint = SpawnPoint;
				}
			}

			UnoccupiedSpawnPoints.Remove(farthestSpawnPoint);
			OccupiedSpawnPoints.Add(farthestSpawnPoint);

			TArray<APlayerStart*> teamSpawnPoints;
			teamSpawnPoints.Add(farthestSpawnPoint);
			TeamToSpawnMap.Add(targetTeam, teamSpawnPoints);
			return farthestSpawnPoint;
		}
	}

	return nullptr;
}
