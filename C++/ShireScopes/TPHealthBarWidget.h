// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomUserWidget.h"
#include "TPHealthBarWidget.generated.h"

/**
 * The widget class used to display health above ShireCharacters.
 * Refer to BP_TPHealthBar for more functionality.
 */

class AShireCharacter;

UCLASS()
class SHIRESCOPES_API UTPHealthBarWidget : public UCustomUserWidget
{
	GENERATED_BODY()

public:

	/**
	 * Set the color of the health bar to blue or red.
	 * @param IsClientTeamate if the owning character of this widget is teamates with the local player on this game client.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetColor(bool IsClientTeammate);

	/** Triggered when this widget's owning character health is changed. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnHealthChange();

	/** Bind to events broadcasted by this widget's owning character. */
	void BindToCharacterEvents(AShireCharacter* TargetCharacter);

protected:

	UPROPERTY(BlueprintReadOnly)
	AShireCharacter* OwningCharacter;

private:
	
};
