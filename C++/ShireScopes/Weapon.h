// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "CooldownData.h"
#include "ShireCharacter.h"
#include "Weapon.generated.h"

class AShireCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActivate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHit, bool, HitPlayerIsTeammate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCriticalHit, bool, HitPlayerIsTeammate);

/**
 * The base class used for all weapons handled by ShireCharacters.
 */

UCLASS(abstract)
class SHIRESCOPES_API AWeapon : public AActor
{
	GENERATED_BODY()

public:

	/** Triggered when this weapon is successfully activated. */
	UPROPERTY(BlueprintAssignable)
	FOnActivate OnActivate;

	/** Triggered when this weapon is detects a hit on a valid target. */
	UPROPERTY(BlueprintAssignable)
	FOnHit OnHit;

	/** Triggered when this weapon is detects a critical hit on a valid target. */
	UPROPERTY(BlueprintAssignable)
	FOnCriticalHit OnCriticalHit;

	/** The struct for managing cooldowns between weapon activations. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	FCooldownData CooldownData;

	/** This weapon's mesh from its associated BP viewport. */
	USkeletalMeshComponent* MeshComponent;

	AWeapon();
	virtual void Tick(float DeltaTime) override;

	/** Attempt to activate this weapon. */
	void AttemptActivate();

	/** Cache the character that owns this weapon. */
	void SetOwningCharacter(AShireCharacter* TargetCharacter);

protected:
	
	/** The maximum distance in which this weapon can register a hit. */
	UPROPERTY(EditDefaultsOnly, Category = "Attributes")
	float Range;

	/** The damage this weapon inflicts upon hitting a valid target. */
	UPROPERTY(EditDefaultsOnly, Category = "Attributes")
	float Damage;

	/** The healing this weapon provides upon hitting a valid target. */
	UPROPERTY(EditDefaultsOnly, Category = "Attributes")
	float Healing;

	/** The character that owns this weapon. */
	UPROPERTY(BlueprintReadOnly)
	AShireCharacter* OwningCharacter;

	/** Enable this weapon to be activated again. */
	UFUNCTION()
	void EnableActivate(bool bEnable);

	virtual void Activate() PURE_VIRTUAL(AWeapon::Activate, );
	virtual void BeginPlay() override;
	

};
