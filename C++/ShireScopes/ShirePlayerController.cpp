// Fill out your copyright notice in the Description page of Project Settings.

#include "ShirePlayerController.h"
#include "ShireCharacter.h"
#include "ShireGameState.h"
#include "ShirePlayerState.h"
#include "ShireHUDWidget.h"
#include "SaveUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "LoadingScreenWidget.h"
#include "ShireGameInstance.h"
#include "ShireSpectator.h"

AShirePlayerController::AShirePlayerController()
{

}

void AShirePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		OnLocalControllerBeginPlay();
		SetInputMode(FInputModeGameOnly());
		ApplyUserSettings();
		BindToGameEvents();
		SetupShireWidgets();
	}
}

void AShirePlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CheckFor_SetupCharacterWidgets();
}

void AShirePlayerController::OnPossess(APawn* aPawn)
{
	// Triggered by Server only.
	Super::OnPossess(aPawn);
}

void AShirePlayerController::AcknowledgePossession(APawn* aPawn)
{
	// Runs on Owning-Client only.
	Super::AcknowledgePossession(aPawn);

	AShireCharacter* shireCharacter = Cast<AShireCharacter>(GetPawn());
	AShireSpectator* shireSpectator = Cast<AShireSpectator>(GetPawn());

	if ( shireCharacter )
	{
		// DEBUG
		UShireGameInstance* gameInstance = Cast<UShireGameInstance>(GetGameInstance());
		if (!gameInstance->Debug_bPreventInputLocks)
		{
			shireCharacter->DisableInput(this);
		}
		// ---

		shireCharacter->OnLocalPlayerAcknowledgePossession(this);
		SetupData.bSetupCharacterWidgets_Complete = false; // Reset upon possessing a new Shire Character.
		shireCharacter->OnDeath.AddDynamic(this, &AShirePlayerController::OnCharacterDeath);
		
		if ( ShireHUDWidget )
		{
			ShireHUDWidget->SetVisibility(ESlateVisibility::Visible);
		}
		
	}
	else if ( shireSpectator && ShireHUDWidget )
	{
		ShireHUDWidget->SetVisibility(ESlateVisibility::Hidden);
	}

}

void AShirePlayerController::OnCharacterDeath(AShireCharacter* DeathCharacter)
{
	FTimerDelegate timerDelegate;
	FTimerHandle unusedHandle;
	timerDelegate.BindUFunction(this, FName(TEXT("Server_SpectateMode")), DeathCharacter);
	GetWorldTimerManager().SetTimer(unusedHandle, timerDelegate, SpectateModeDelay, false);
}

void AShirePlayerController::Server_SpectateMode_Implementation(AShireCharacter* DeathCharacter)
{
	// Spawn spectator at the TP Camera transform.
	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	FTransform CameraTransformTP = DeathCharacter->GetThirdPersonCameraTransform();
	AShireSpectator* spectatorPawn = Cast<AShireSpectator>(GetWorld()->SpawnActor<AActor>(AShireSpectator::StaticClass(), CameraTransformTP, spawnParams));

	// Possess spectator pawn.
	Possess(spectatorPawn);
}

void AShirePlayerController::OnRoundStart()
{
	GetPawn()->EnableInput(this);
}

void AShirePlayerController::OnMatchOver(int WinningTeam)
{
	ShowGameStats();
}

void AShirePlayerController::OnLocalControllerBeginPlay()
{
	AShireGameState* shireGameState = Cast<AShireGameState>(UGameplayStatics::GetGameState(GetWorld()));
	shireGameState->SetLocalPlayer(this);
}

void AShirePlayerController::ShowGameStats()
{
	if ( GameStatsWidget )
	{
		GameStatsWidget->SetVisibility(ESlateVisibility::Visible);
	}	
}

void AShirePlayerController::HideGameStats()
{
	if (GameStatsWidget)
	{
		GameStatsWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AShirePlayerController::BindToGameEvents()
{
	AShireGameState* shireGameState = Cast<AShireGameState>(UGameplayStatics::GetGameState(GetWorld()));
	shireGameState->OnRoundStart.AddDynamic(this, &AShirePlayerController::OnRoundStart);
	shireGameState->OnMatchOver.AddDynamic(this, &AShirePlayerController::OnMatchOver);
}

void AShirePlayerController::CheckFor_SetupCharacterWidgets()
{
	if ( !SetupData.bSetupCharacterWidgets_Complete && GetPawn() && ShireHUDWidget && ScopeWidget )
	{
		AShireCharacter* shireCharacter = Cast<AShireCharacter>(GetPawn());
		shireCharacter->SetupWidgets(ShireHUDWidget, ScopeWidget);
		SetupData.bSetupCharacterWidgets_Complete = true;
	}
}

void AShirePlayerController::ToggleExitMenu()
{
	if ( ExitMenuWidget->IsVisible() )
	{
		SetInputMode(FInputModeGameOnly());
		SetShowMouseCursor(false);
		ExitMenuWidget->SetVisibility(ESlateVisibility::Hidden);
		ShireHUDWidget->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		SetInputMode(FInputModeGameAndUI());
		SetShowMouseCursor(true);
		ExitMenuWidget->SetVisibility(ESlateVisibility::Visible);
		ShireHUDWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AShirePlayerController::ToggleLookSensitivity(bool bUseScopeSensitivity)
{
	if (bUseScopeSensitivity)
	{
		SetDeprecatedInputYawScale(ScopeSensitivity);
		SetDeprecatedInputPitchScale(-ScopeSensitivity);
	}
	else
	{
		SetDeprecatedInputYawScale(LookSensitivity);
		SetDeprecatedInputPitchScale(-LookSensitivity);
	}
}

void AShirePlayerController::ApplyUserSettings()
{
	USaveUserSettings* userSettings = Cast<USaveUserSettings>(UGameplayStatics::LoadGameFromSlot("User Settings", 0));

	if (userSettings)
	{
		LookSensitivity = userSettings->LookSensitivity * SensitivityScalar;
		ScopeSensitivity = userSettings->ScopeSensitivity * SensitivityScalar;
	}

	SetDeprecatedInputYawScale(LookSensitivity);
	SetDeprecatedInputPitchScale(-LookSensitivity);

}


void AShirePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("Controller::ToggleGameStats", IE_Pressed, this, &AShirePlayerController::ShowGameStats);
	InputComponent->BindAction("Controller::ToggleGameStats", IE_Released, this, &AShirePlayerController::HideGameStats);
	InputComponent->BindAction("Controller::ToggleExitMenu", IE_Pressed, this, &AShirePlayerController::ToggleExitMenu);

	InputComponent->BindAction("Keyboard::ToggleGameStats", IE_Pressed, this, &AShirePlayerController::ShowGameStats);
	InputComponent->BindAction("Keyboard::ToggleGameStats", IE_Released, this, &AShirePlayerController::HideGameStats);
	InputComponent->BindAction("Keyboard::ToggleExitMenu", IE_Pressed, this, &AShirePlayerController::ToggleExitMenu);
}

void AShirePlayerController::SetupGameWidgets()
{
	Super::SetupGameWidgets();
	Client_ToggleLoadingScreen(true, FString(TEXT("Waiting For Players")));
}

void AShirePlayerController::SetupShireWidgets()
{
	ShireHUDWidget = Cast<UShireHUDWidget>(CreateWidget(this, ShireHUDWidgetClass));
	ShireHUDWidget->AddToViewport();

	ScopeWidget = CreateWidget(this, ScopeWidgetClass);
	ScopeWidget->AddToViewport();
	ScopeWidget->SetVisibility(ESlateVisibility::Hidden);

	GameStatsWidget = CreateWidget(this, GameStatsWidgetClass);
	GameStatsWidget->AddToViewport();
	GameStatsWidget->SetVisibility(ESlateVisibility::Hidden);

	GameHUDWidget = Cast<UShireHUDWidget>(CreateWidget(this, GameHUDWidgetClass));
	GameHUDWidget->AddToViewport();

	ExitMenuWidget = CreateWidget(this, ExitMenuWidgetClass);
	ExitMenuWidget->AddToViewport();
	ExitMenuWidget->SetVisibility(ESlateVisibility::Hidden);
}
