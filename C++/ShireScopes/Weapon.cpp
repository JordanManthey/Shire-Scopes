// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	// this might need to come before super...
	TArray<USkeletalMeshComponent*> Components;
	GetComponents<USkeletalMeshComponent>(Components);
	for (int i = 0; i < Components.Num(); i++)
	{
		MeshComponent = Components[i];
	}
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeapon::EnableActivate(bool bEnable)
{
	CooldownData.bEnabled = bEnable;
}

void AWeapon::AttemptActivate()
{
	if ( !CooldownData.bEnabled || !HasActorBegunPlay() ) { return; }

	EnableActivate(false);
	OnActivate.Broadcast();

	FTimerDelegate enableActivate;
	enableActivate.BindUFunction(this, FName("EnableActivate"), true);
	GetWorldTimerManager().SetTimer(CooldownData.Timer, enableActivate, CooldownData.Duration, false, CooldownData.Duration);

	Activate();

}

void AWeapon::SetOwningCharacter(AShireCharacter* TargetCharacter)
{
	OwningCharacter = TargetCharacter;
	SetOwner(OwningCharacter);
}

