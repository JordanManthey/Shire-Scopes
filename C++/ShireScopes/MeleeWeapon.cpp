// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeWeapon.h"
#include "DrawDebugHelpers.h" // TEST

void AMeleeWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void AMeleeWeapon::Activate()
{
	// Activate triggers Character Melee State. Then Swing AnimNotify calls Swing(). 
}

void AMeleeWeapon::Swing()
{
	if (!OwningCharacter || !OwningCharacter->IsLocallyControlled()) { return; }

	FVector playerViewLocation;
	FRotator playerViewRotation;
	FVector raycastEndTrace = FVector::ZeroVector;

	OwningCharacter->GetController()->GetPlayerViewPoint(playerViewLocation, playerViewRotation);
	raycastEndTrace = playerViewLocation + (playerViewRotation.Vector() * Range);

	FCollisionQueryParams raycastTraceParams(SCENE_QUERY_STAT(RangedWeaponRaycast), true, OwningCharacter);
	FHitResult raycastHit(ForceInit);
	FCollisionShape sweepShape = FCollisionShape::MakeBox(FVector(10, 50, 20));
	GetWorld()->SweepSingleByChannel(raycastHit, playerViewLocation, raycastEndTrace, playerViewRotation.Quaternion(), ECC_Visibility, sweepShape, raycastTraceParams);
	//DrawDebugBox(GetWorld(), raycastEndTrace, sweepShape.GetExtent(), playerViewRotation.Quaternion(), FColor::Cyan, true);

	// Process hit
	AShireCharacter* hitPlayer = Cast<AShireCharacter>(raycastHit.GetActor());

	if (hitPlayer)
	{
		bool hitPlayerIsTeammate = OwningCharacter->IsTeammate(hitPlayer);
		OnHit.Broadcast(hitPlayerIsTeammate);
		OwningCharacter->Server_RegisterHit(hitPlayer, Damage, Healing, playerViewRotation.Vector());
	}
}
