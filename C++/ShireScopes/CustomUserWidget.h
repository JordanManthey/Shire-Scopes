// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CustomUserWidget.generated.h"

/**
 * The base class for any ShireScopes UserWidget nonnative to UE source.
 * Ensures that the seamless travel flag exists for proper filtering downstream.
 */

UCLASS()
class SHIRESCOPES_API UCustomUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	bool bPersistedBySeamlessTravel{ false };

protected:

private:
	
};
