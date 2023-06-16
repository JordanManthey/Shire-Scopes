// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseState.h"

/**
 * This is the Fire state that a ShireCharacter can take when firing its ranged weapon.
 * Refer to its abstract parent class BaseState for more details.
 */

class SHIRESCOPES_API FireState : public BaseState
{
public:
	FireState();
	~FireState();
	bool AllowsNextState(EState stateName) const override;

protected:


};

