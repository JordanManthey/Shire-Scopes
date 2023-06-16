// Fill out your copyright notice in the Description page of Project Settings.

#include "CustomPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "SaveUserSettings.h"
#include "LoadingScreenWidget.h"
#include "GameMessageWidget.h"
#include "CustomGameMode.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/PlayerState.h"
#include "ShireCharacter.h"
#include "ShireGameInstance.h"

ACustomPlayerController::ACustomPlayerController()
{
}

void ACustomPlayerController::PostInitProperties()
{
	Super::PostInitProperties();
}

void ACustomPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void ACustomPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CheckFor_LocalPossessionAndBeginPlay();
	CheckFor_FilterSeamlessTravelWidgets();
}

void ACustomPlayerController::AcknowledgePossession(APawn* aPawn)
{
	Super::AcknowledgePossession(aPawn);
}

void ACustomPlayerController::Server_OnLocalControllerPossessAndBeginPlay_Implementation() const
{
	ACustomGameMode* gameMode = Cast<ACustomGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	gameMode->OnLocalControllerPossessAndBeginPlay();
}

void ACustomPlayerController::Client_TravelToLevel_Implementation(const FString& LevelName, const FString& LoadingMessage, const FString& Options)
{
	Client_ToggleLoadingScreen(true, LoadingMessage);
	UShireGameInstance* gameInstance = Cast<UShireGameInstance>(GetGameInstance());
	gameInstance->EndClientSession(this);
	UGameplayStatics::OpenLevel(GetWorld(), FName(LevelName), true, Options);
}

void ACustomPlayerController::Client_ToggleLoadingScreen_Implementation(bool bIsVisible, const FString& LoadingMessage) const
{
	if (LoadingScreenWidget)
	{
		LoadingScreenWidget->SetLoadingMessage(LoadingMessage);

		if (bIsVisible)
		{
			LoadingScreenWidget->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			LoadingScreenWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ACustomPlayerController::Client_ShowGameMessage_Implementation(const FString& Message) const
{
	if (GameMessageWidget)
	{
		GameMessageWidget->SetMessage(Message);
		GameMessageWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void ACustomPlayerController::Client_ShowGameMessageFromOptions_Implementation(const FString& OptionsString) const
{
	if (GameMessageWidget)
	{
		FString targetMessage = GameMessageWidget->GetMessageFromOptions(OptionsString);
		if (!targetMessage.IsEmpty()) // If there is a message associated with this optionString.
		{
			Client_ShowGameMessage(targetMessage);
		}
	}
}

void ACustomPlayerController::Server_HandleGameMessageFromOptions_Implementation() const
{
	AGameModeBase* gameMode = UGameplayStatics::GetGameMode(GetWorld());
	Client_ShowGameMessageFromOptions(gameMode->OptionsString);
}

void ACustomPlayerController::ApplyUserSettings()
{
	// Define in child class.
}

void ACustomPlayerController::FilterSeamlessTravelWidgets() const 
{
	TArray<UUserWidget*> widgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), widgets, UCustomUserWidget::StaticClass(), true);
	for (int i = 0; i < widgets.Num(); i++)
	{
		UCustomUserWidget* widget = Cast<UCustomUserWidget>(widgets[i]);
		if ( !WidgetsToPersistOnSeamlessTravel.Contains(widget->GetClass()) ) // If this is not on our list of widget classes to persist.
		{
			if ( widget->GetOwningPlayer()->PlayerState->PlayerId == PlayerState->PlayerId ) // If this widget is owned by this PC.
			{
				if ( widget->bPersistedBySeamlessTravel ) // If this widget persisted from before a seamless travel.
				{
					widget->RemoveFromViewport();
				}
			}
		}
	}
}

void ACustomPlayerController::SetupGameWidgets()
{
	// CONCERN: If Client triggers this before being possessed, the RPCs to the Server (and back) could not work.
	if (GameMessageWidgetClass)
	{
		GameMessageWidget = Cast<UGameMessageWidget>(CreateWidget(this, GameMessageWidgetClass));
	}

	if (LoadingScreenWidgetClass)
	{
		LoadingScreenWidget = Cast<ULoadingScreenWidget>(CreateWidget(this, LoadingScreenWidgetClass));
	}

	if (GameMessageWidget)
	{
		GameMessageWidget->AddToViewport(1);
	}

	if (LoadingScreenWidget)
	{
		LoadingScreenWidget->AddToViewport(1);
		Client_ToggleLoadingScreen(false);
	}

	Server_HandleGameMessageFromOptions();
}

void ACustomPlayerController::CheckFor_LocalPossessionAndBeginPlay()
{
	// Waiting for BeginPlay to ensure GameMessageWidget has access to GameState downstream.
	if ( !SetupData.LocalPossessionAndBeginPlay_Complete && IsLocalController() && HasActorBegunPlay() )
	{
		SetupGameWidgets();
		Server_OnLocalControllerPossessAndBeginPlay();
		SetupData.LocalPossessionAndBeginPlay_Complete = true;
	}
}

void ACustomPlayerController::CheckFor_FilterSeamlessTravelWidgets()
{
	if ( !SetupData.FilterSeamlessTravelWidgets_Complete && PlayerState )
	{
		FilterSeamlessTravelWidgets();
		SetupData.FilterSeamlessTravelWidgets_Complete = true;
	}
}
