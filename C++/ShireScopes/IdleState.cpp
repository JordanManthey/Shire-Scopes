// Fill out your copyright notice in the Description page of Project Settings.


#include "IdleState.h"

IdleState::IdleState()
{
	StateName = EState::IDLE;
}

IdleState::~IdleState()
{
}

bool IdleState::AllowsNextState(EState stateName) const
{
	switch (stateName) {
	case EState::IDLE:
		return false;
		break;
	case EState::AIM:
		return true;
		break;
	case EState::MELEE:
		return true;
		break;
	case EState::FIRE:
		return true;
		break;
	case EState::EQUIP:
		return true;
		break;
	default:
		return false;
	}
}
