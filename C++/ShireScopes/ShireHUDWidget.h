// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomUserWidget.h"
#include "ShireHUDWidget.generated.h"

class AShireCharacter;

/**
 * The HUD class for a player, which ultimately needs to be linked to an owning ShireCharacter.
 * Refer to BP_ShireHUDWidget for more functionality.
 */

UCLASS()
class SHIRESCOPES_API UShireHUDWidget : public UCustomUserWidget
{
	GENERATED_BODY()
	
public:

	void SetCharacter(AShireCharacter* TargetCharacter);

protected:

	UPROPERTY(BlueprintReadOnly)
	AShireCharacter* Character;

private:

};
