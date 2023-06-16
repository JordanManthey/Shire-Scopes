// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "RangedWeapon.generated.h"

class UNiagaraComponent;
class UMaterialInterface;

//////////////////////////////////////////////////////////////////////////
// STRUCT DECLARATIONS
//////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FRecoilData {

	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly)
	float KickStrength{ 5 };

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	float KickDuration{ 1 };

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	float RecoveryDuration{ 1 };

};

//////////////////////////////////////////////////////////////////////////
// GENERAL
//////////////////////////////////////////////////////////////////////////

/**
 * The base class for all ranged weapons.
 */
UCLASS()
class SHIRESCOPES_API ARangedWeapon : public AWeapon
{
	GENERATED_BODY()

public:

	/** The struct used to determined a RangedWeapon's recoil behavior when fired. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	FRecoilData RecoilData;

	void Activate() override;

protected:

	/** The max angle (in degrees) the activation hitscan can deviate. */
	UPROPERTY(EditDefaultsOnly, Category = "Attributes")
	float MaxHipfireSpread{ 10 };

	/** The damage multipler for headshots with this weapon. */
	UPROPERTY(EditDefaultsOnly, Category = "Attributes")
	float HeadshotMultiplier{ 3 };

	/** The material used for spawning decals at the point of raycast impact upon firing. */
	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* ProjectileImpactMaterial;

	/** The size scale of raycast impact decals upon firing. */
	UPROPERTY(EditDefaultsOnly)
	FVector ProjectileImpactScale;

	/** The distance from the barrel at which the projectile trail particles should begin. */
	UPROPERTY(EditDefaultsOnly)
	float ProjectileTrailStartDistance;

	/** The projectile trail actor to spawn upon firing. */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> ProjectileTrailClass;

	virtual void BeginPlay() override;

	/**
	 * The implementation method for how hit detection is handled.
	 * Default method is a raycast hit.
	 */
	void Fire();

private:

};
