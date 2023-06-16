// Fill out your copyright notice in the Description page of Project Settings.

#include "GameMessageWidget.h"
#include "Kismet/GameplayStatics.h"
#include "ShireGameState.h"
#include "ShireGameInstance.h"
#include "LobbyGameState.h"

void UGameMessageWidget::SetMessage_Implementation(const FString& Message)
{
	// Defined in BP.
}

FString UGameMessageWidget::GetMessageFromOptions(const FString& OptionsString)
{
	FString targetMessage = TEXT("");
	FString messageKey = UGameplayStatics::ParseOption(OptionsString, TEXT("gameMessage"));
	FString* optionsMessage = GameMessageMap.Find(messageKey);

	if (!ShouldIgnoreMessage(OptionsString) && optionsMessage)
	{
		targetMessage = *optionsMessage;
	}

	return targetMessage;
}

bool UGameMessageWidget::ShouldIgnoreMessage(const FString& OptionsString)
{
	FString messageKey = UGameplayStatics::ParseOption(OptionsString, TEXT("gameMessage"));
	FString travelType = UGameplayStatics::ParseOption(OptionsString, TEXT("travelType"));
	bool isHost = UGameplayStatics::GetPlayerController(GetWorld(), 0)->HasAuthority();

	// Prevent Server from recieving "hostless" messages after server travel.
	if (isHost && travelType == TEXT("server") && messageKey == TEXT("hostless"))
	{
		return true;
	}

	// Prevent Client from recieving "hostless" messages if not returning to lobby from a match.
	UShireGameInstance* shireGameInstance = Cast<UShireGameInstance>(GetGameInstance());
	UClass* previousGameStateClass = shireGameInstance->PreviousGameStateClass;
	UClass* currentGameStateClass = UGameplayStatics::GetGameState(GetWorld())->GetClass();

	bool playerIsInLobby = currentGameStateClass->IsChildOf(ALobbyGameState::StaticClass());
	bool playerCameFromMatch = false;

	if ( previousGameStateClass )
	{
		playerCameFromMatch = previousGameStateClass->IsChildOf(AShireGameState::StaticClass());
	}

	if ( playerIsInLobby && !playerCameFromMatch && messageKey == TEXT("hostless") )
	{
		return true;
	}

	return false;

}
