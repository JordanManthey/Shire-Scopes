// Fill out your copyright notice in the Description page of Project Settings.

#include "FireState.h"

FireState::FireState()
{
	StateName = EState::FIRE;
}

FireState::~FireState()
{
}

bool FireState::AllowsNextState(EState stateName) const
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

