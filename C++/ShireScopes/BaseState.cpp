// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseState.h"

BaseState::BaseState()
{
}

BaseState::~BaseState()
{
}

EState BaseState::GetStateName() const
{
	return StateName;
}
