// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomUserWidget.h"
#include "GameMessageWidget.generated.h"

/**
 * The widget class used for displaying game messages to the player.
 */

UCLASS()
class SHIRESCOPES_API UGameMessageWidget : public UCustomUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void SetMessage(const FString& Message);

	void SetMessage_Implementation(const FString& Message);
	FString GetMessageFromOptions(const FString& OptionsString);

protected:

	/** A map of OptionsString keys to the associated game message to be displayed. */
	UPROPERTY(EditDefaultsOnly)
	TMap<FString, FString> GameMessageMap;

	/**
	 * @param OptionsString from the GameMode.
	 * @returns true if the GameMessage should not be shown to this player.
	 */
	bool ShouldIgnoreMessage(const FString& OptionsString);

private:

};
