// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShireGameMode.h"
#include "EliminationGameMode.generated.h"

/**
 * A round-based GameMode in which the last team alive is the winner of each round.
 */

UCLASS()
class SHIRESCOPES_API AEliminationGameMode : public AShireGameMode
{
	GENERATED_BODY()

public:

protected:

	virtual void OnPlayerDeath(AShireCharacter* DeathCharacter) override;
	virtual int GetRoundWinningTeam() const override;
	virtual bool AnyTeamHasWonRound() const override;

private:
	
};
