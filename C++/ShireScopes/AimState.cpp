// Fill out your copyright notice in the Description page of Project Settings.


#include "AimState.h"

AimState::AimState()
{
	StateName = EState::AIM;
}

AimState::~AimState()
{
}

bool AimState::AllowsNextState(EState stateName) const
{
	switch (stateName) {
	case EState::IDLE:
		return true;
		break;
	case EState::AIM:
		return false;
		break;
	case EState::MELEE:
		return false;
		break;
	case EState::FIRE:
		return true;
		break;
	case EState::EQUIP:
		return false;
		break;
	default:
		return false;
	}
}
