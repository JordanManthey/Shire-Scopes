// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "MeleeWeapon.generated.h"

/**
 * The base class for all melee weapons.
 */

UCLASS()
class SHIRESCOPES_API AMeleeWeapon : public AWeapon
{
	GENERATED_BODY()
	

public:

	void Activate() override;

protected:

	/** 
	 * The implementation method for how hit detection is handled. 
	 * Default method is a volume raycast sweep.
	 */
	UFUNCTION(BlueprintCallable)
	void Swing();

	virtual void BeginPlay() override;

private:


};
