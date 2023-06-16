// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SaveUserSettings.generated.h"

/**
 * The class used for saving custom user settings.
 */
UCLASS()
class SHIRESCOPES_API USaveUserSettings : public USaveGame
{
	GENERATED_BODY()

public:

	/** The player's sensitivity when looking around. */
	UPROPERTY(BlueprintReadWrite)
	float LookSensitivity{ 0.5 };

	/** The player's sensitivity when scoped into their RangedWeapon and looking around. */
	UPROPERTY(BlueprintReadWrite)
	float ScopeSensitivity{ 0.5 };

protected:

private:
	
};
