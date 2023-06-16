// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseState.h"

/**
 * This is the Equip state that a ShireCharacter can take when equiping/changing weapons.
 * Refer to its abstract parent class BaseState for more details.
 */

class SHIRESCOPES_API EquipState : public BaseState
{
public:
	EquipState();
	~EquipState();
	bool AllowsNextState(EState stateName) const override;

protected:


};

