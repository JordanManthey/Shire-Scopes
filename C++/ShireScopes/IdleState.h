// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseState.h"

/**
 * This is the Idle state that a ShireCharacter can take when not taking on any other state.
 * Refer to its abstract parent class BaseState for more details.
 */

class SHIRESCOPES_API IdleState : public BaseState
{
public:
	IdleState();
	~IdleState();
	bool AllowsNextState(EState stateName) const override;
	
protected:
	

};

