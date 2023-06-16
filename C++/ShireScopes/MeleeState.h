// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseState.h"

/**
 * A roundless GameMode with no winner. Can only be ended by the Host.
 * Players respawn after death indefinitely.
 */

class SHIRESCOPES_API MeleeState : public BaseState
{
public:
	MeleeState();
	~MeleeState();
	bool AllowsNextState(EState stateName) const override;
};
