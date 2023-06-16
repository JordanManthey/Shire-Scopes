// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseState.h"

/**
 * This is the Aim state that a ShireCharacter can take when aiming its ranged weapon.
 * Refer to its abstract parent class BaseState for more details.
 */

class SHIRESCOPES_API AimState : public BaseState
{
public:

	AimState();
	~AimState();
	bool AllowsNextState(EState stateName) const override;

protected:


};
