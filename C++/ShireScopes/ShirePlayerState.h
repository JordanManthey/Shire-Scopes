// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ShirePlayerState.generated.h"

/**
 * This is the base class for all PlayerStates associated with a ShirePlayerController.
 */

UCLASS()
class SHIRESCOPES_API AShirePlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	UPROPERTY(Replicated, BlueprintReadWrite)
	int Team{ 1 };

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	bool IsTeammate(AShirePlayerState* OtherPlayer);

protected:

	virtual void BeginPlay() override;
	virtual void CopyProperties(APlayerState* PlayerState) override;

private:
	

};
