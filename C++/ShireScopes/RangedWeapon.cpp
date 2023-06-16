// Fill out your copyright notice in the Description page of Project Settings.

#include "RangedWeapon.h"
#include "NiagaraComponent.h"
#include "ShireAIController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DecalComponent.h"

void ARangedWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void ARangedWeapon::Activate()
{
	Fire();
}

void ARangedWeapon::Fire()
{
	if (!OwningCharacter) { return; }

	FVector playerViewLocation;
	FRotator playerViewRotation;
	FVector raycastEndTrace = FVector::ZeroVector;

	// Apply bullet spread differently for players and bots.
	float spreadStrength;
	if ( !OwningCharacter->IsPlayerControlled() )
	{
		AShireAIController* botController = Cast<AShireAIController>(OwningCharacter->GetController());
		spreadStrength = botController->GetBulletSpreadStrength();
	}
	else
	{
		spreadStrength = std::max(0.0f, OwningCharacter->ToggleScopePercentThreshold - OwningCharacter->ADSPercent);
	}
	
	OwningCharacter->GetController()->GetPlayerViewPoint(playerViewLocation, playerViewRotation);
	
	playerViewRotation.Pitch += FMath::RandRange(-MaxHipfireSpread, MaxHipfireSpread) * spreadStrength;
	playerViewRotation.Yaw += FMath::RandRange(-MaxHipfireSpread, MaxHipfireSpread) * spreadStrength;
	raycastEndTrace = playerViewLocation + (playerViewRotation.Vector() * Range);

	FCollisionQueryParams raycastTraceParams(SCENE_QUERY_STAT(RangedWeaponRaycast), true, OwningCharacter);
	FHitResult raycastHit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(raycastHit, playerViewLocation, raycastEndTrace, ECC_Visibility, raycastTraceParams);
	//DrawDebugLine(GetWorld(), playerViewLocation, raycastEndTrace, FColor::Cyan, true);

	// Process hit
	AShireCharacter* hitPlayer = Cast<AShireCharacter>(raycastHit.GetActor());

	if (hitPlayer)
	{
		bool hitPlayerIsTeammate = OwningCharacter->IsTeammate(hitPlayer);
		float targetDamage = Damage;
		float targetHealing = Healing;

		if (raycastHit.BoneName == "Head")
		{
			targetDamage = Damage * HeadshotMultiplier;
			targetHealing = Healing * HeadshotMultiplier;
			OnCriticalHit.Broadcast(hitPlayerIsTeammate);
		}
		else
		{
			OnHit.Broadcast(hitPlayerIsTeammate);
		}

		OwningCharacter->Server_RegisterHit(hitPlayer, targetDamage, targetHealing, playerViewRotation.Vector());
	}

	// Spawn bullet trail if player ADS
	if (OwningCharacter->ADSPercent > OwningCharacter->ToggleScopePercentThreshold)
	{
		//OwningCharacter->SpawnProjectileTrail(ProjectileTrailClass, playerViewLocation, playerViewRotation, ProjectileTrailStartDistance);
		//OwningCharacter->Server_SpawnProjectileTrail(ProjectileTrailClass, playerViewLocation, playerViewRotation, ProjectileTrailStartDistance);
	}

	// Spawn impact decal
	UDecalComponent* impactDecal = UGameplayStatics::SpawnDecalAttached(ProjectileImpactMaterial, ProjectileImpactScale, raycastHit.GetComponent(), NAME_None, raycastHit.ImpactPoint, raycastHit.ImpactNormal.Rotation(), EAttachLocation::KeepWorldPosition, 20);
	if ( impactDecal )
	{
		impactDecal->SetFadeScreenSize(0.001);
	}
	
}
