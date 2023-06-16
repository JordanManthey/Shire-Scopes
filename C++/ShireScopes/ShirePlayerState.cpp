// Fill out your copyright notice in the Description page of Project Settings.

#include "ShirePlayerState.h"
#include "ShireGameInstance.h"
#include "Net/UnrealNetwork.h"

void AShirePlayerState::BeginPlay()
{
	Super::BeginPlay();
}

void AShirePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShirePlayerState, Team);
}

void AShirePlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);
	AShirePlayerState* targetPlayerState = Cast<AShirePlayerState>(PlayerState);
	targetPlayerState->Team = Team;
}

bool AShirePlayerState::IsTeammate(AShirePlayerState* OtherPlayer)
{
	if (Team == OtherPlayer->Team)
	{
		return true;
	}
	return false;
}
