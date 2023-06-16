// Fill out your copyright notice in the Description page of Project Settings.

#include "TPHealthBarWidget.h"
#include "ShireCharacter.h"

void UTPHealthBarWidget::SetColor_Implementation(bool IsClientTeammate)
{
	// Defined in BP.
}

void UTPHealthBarWidget::OnHealthChange_Implementation()
{
	// Defined in BP.
}

void UTPHealthBarWidget::BindToCharacterEvents(AShireCharacter* TargetCharacter)
{
	OwningCharacter = TargetCharacter;
	OwningCharacter->OnHealthChange.AddDynamic(this, &UTPHealthBarWidget::OnHealthChange);
}
