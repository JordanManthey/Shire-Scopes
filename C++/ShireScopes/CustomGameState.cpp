// Fill out your copyright notice in the Description page of Project Settings.

#include "CustomGameState.h"
#include "CustomUserWidget.h"
#include "ShireGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

void ACustomGameState::Multicast_FlagSeamlessTravelWidgets_Implementation() const
{
	TArray<UUserWidget*> widgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), widgets, UCustomUserWidget::StaticClass(), true);
	for (int i = 0; i < widgets.Num(); i++)
	{
		UCustomUserWidget* widget = Cast<UCustomUserWidget>(widgets[i]);
		widget->bPersistedBySeamlessTravel = true;
	}
}

void ACustomGameState::Multicast_CacheGameStateClass_Implementation() const
{
	UShireGameInstance* shireGameInstance = Cast<UShireGameInstance>(GetGameInstance());
	if (shireGameInstance)
	{
		shireGameInstance->CacheGameStateClass();
	}
}

void ACustomGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
	OnPlayerArrayUpdate.Broadcast();
}

void ACustomGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);
	OnPlayerArrayUpdate.Broadcast();
}

void ACustomGameState::OnPlayerLogout(APlayerController* Player)
{
	// Define in child class.
}

bool  ACustomGameState::IsBotID(int32 ID)
{
	return (ID > 1000);
}
