// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeState.h"

MeleeState::MeleeState()
{
	StateName = EState::MELEE;
}

MeleeState::~MeleeState()
{
}

bool MeleeState::AllowsNextState(EState stateName) const
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
		return false;
		break;
	case EState::EQUIP:
		return false;
		break;
	default:
		return false;
	}
}
