// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomUserWidget.h"
#include "LoadingScreenWidget.generated.h"

/**
 * The widget class used for displaying loading messages to the player.
 */

UCLASS()
class SHIRESCOPES_API ULoadingScreenWidget : public UCustomUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetLoadingMessage(const FString& Message);

	void SetLoadingMessage_Implementation(const FString& Message);

protected:

private:
};
