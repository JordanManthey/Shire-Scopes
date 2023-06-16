// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CooldownData.generated.h"

/**
 * This is the based struct used for various ShireCharacter cooldown abilties.
 */
USTRUCT(BlueprintType)
struct FCooldownData {

	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Duration{ 1.0 };

	FTimerHandle Timer;

	bool bEnabled{ true };

};
