// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterStateManagerComponent.h"

/**
 * This is the base abstract class for all primary states of the ShireCharacter, which is managed by CharacterStateManagerComponent.
 * Used to determine the current state and valid transition states.
 */

class SHIRESCOPES_API BaseState
{

public:
	BaseState();
	~BaseState();

	/** @returns if the given state can be transitioned to from this state. */
	 virtual bool AllowsNextState(EState stateName) const = 0;

	EState GetStateName() const;
	EState StateName;

protected:
	
};
