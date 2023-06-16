// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShireCharacter.h"
#include "ShirePlayerController.h"
#include "ShireGameMode.h"
#include "ShireHUDWidget.h"
#include "ShireGameState.h"
#include "ShirePlayerState.h"
#include "ShireAIController.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/WidgetComponent.h"
#include "NiagaraComponent.h"
#include "Camera/CameraComponent.h"
#include "CharacterStateManagerComponent.h"
#include "Perception/AISense_Damage.h"
#include "RangedWeapon.h"
#include "MeleeWeapon.h"
#include "TPHealthBarWidget.h"
#include "ShireGameInstance.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GENERAL
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AShireCharacter::AShireCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	TurnRateGamepad = 45.f;

	// Create a Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a State Manager Component
	StateManager = CreateDefaultSubobject<UCharacterStateManagerComponent>("Character State Manager");

	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);

	MovementComponent = Cast<UCharacterMovementComponent>(GetCharacterMovement());

}

void AShireCharacter::BeginPlay()
{
	// Call before Super::BeginPlay in order to execute before BP's BeginPlay()
	SetupComponentReferences();
	SetupRecoilTimeline();

	Super::BeginPlay();

	WalkSpeed = MovementComponent->MaxWalkSpeed;
	BaseGroundFriction = MovementComponent->GroundFriction;
	BaseCapsuleHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();

	Health = MaxHealth;
	BaseMeshLocation = MeshTP->GetRelativeLocation();
	BaseCameraLocation = FirstPersonCameraComponent->GetRelativeLocation();

}

void AShireCharacter::Tick(float DeltaTime) {

	Super::Tick(DeltaTime);
	
	CheckFor_GameStateLocalPlayer();
	CheckFor_ClientLocalPlayerStateAndPlayerState();
	CheckFor_LocalPossessionAndBeginPlay();
	CheckFor_NonLocalThirdPersonSetup();
	CheckFor_WallJumpReset();
	CheckFor_SlideCancel(DeltaTime);

	UpdateSlideCamera(DeltaTime);
	UpdateHealthBarRotation();

}

EState AShireCharacter::GetPlayerStateName() const
{
	return StateManager->GetCurrentStateName();
}

void AShireCharacter::Multicast_OnPossessed_Implementation()
{
	// WARNING: Will not reliably multicast to clients at level start as the client characters might not be instantiated yet.
	// Use reliably on repossession scenarios.

	AShireGameState* gameState = Cast<AShireGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (gameState && gameState->LocalPlayer)
	{
		ClientLocalPlayer = gameState->LocalPlayer;
	}
}

void AShireCharacter::SetupComponentReferences()
{
	MeshTP = GetMesh();

	UChildActorComponent* rangedWeaponComp = nullptr;
    UChildActorComponent* meleeWeaponComp = nullptr;

	TArray<USceneComponent*> allComponents;
	GetCapsuleComponent()->GetChildrenComponents(true, allComponents);
	for (int i = 0; i < allComponents.Num(); i++) {

		USceneComponent* currComp = allComponents[i];

		if (currComp->GetName() == "Mesh FP") { Mesh1P = Cast<USkeletalMeshComponent>(currComp); }
		if (currComp->GetName() == "Third Person Camera Component") { ThirdPersonCameraComponent = Cast<UCameraComponent>(currComp); }
		if (currComp->GetName() == "Musket Mesh") { RangedWeaponTP = Cast<USkeletalMeshComponent>(currComp); }
		if (currComp->GetName() == "Sword Mesh") { MeleeWeaponTP = Cast<USkeletalMeshComponent>(currComp); }
		if (currComp->GetName() == "BP Musket") { rangedWeaponComp = Cast<UChildActorComponent>(currComp); }
		if (currComp->GetName() == "BP Sword") { meleeWeaponComp = Cast<UChildActorComponent>(currComp); }
		if (currComp->GetName() == "Health Bar TP") { TPHealthBarComponent = Cast<UWidgetComponent>(currComp); }

		if (rangedWeaponComp && !RangedWeapon)
		{
			RangedWeapon = Cast<ARangedWeapon>(rangedWeaponComp->GetChildActor());
			RangedWeapon->SetOwningCharacter(this);
			RangedWeapon->OnActivate.AddDynamic(this, &AShireCharacter::OnRangedWeaponActivate);
			EquippedWeapon = RangedWeapon;
		}

		if (meleeWeaponComp && !MeleeWeapon)
		{
			MeleeWeapon = Cast<AMeleeWeapon>(meleeWeaponComp->GetChildActor());
			MeleeWeapon->SetOwningCharacter(this);
			MeleeWeapon->OnActivate.AddDynamic(this, &AShireCharacter::OnMeleeWeaponActivate);
		}
	}
}

void AShireCharacter::OnAcknowledgePlayerState()
{
	AShirePlayerState* myPlayerState = Cast<AShirePlayerState>(GetPlayerState());
	Team = myPlayerState->Team;

	// DEBUG
	if (Cast<UShireGameInstance>(GetGameInstance())->Debug_bGodMode && IsPlayerControlled() && GetLocalRole() == ROLE_Authority)
	{
		MaxHealth = 10000;
		Health = MaxHealth;
	}
}

void AShireCharacter::CheckFor_GameStateLocalPlayer()
{
	if ( !ClientLocalPlayer && UGameplayStatics::GetGameState(GetWorld()) )
	{
		AShireGameState* gameState = Cast<AShireGameState>(UGameplayStatics::GetGameState(GetWorld()));
		if (gameState->LocalPlayer)
		{
			ClientLocalPlayer = gameState->LocalPlayer;
		}
	}

}

void AShireCharacter::CheckFor_LocalPossessionAndBeginPlay()
{
	if ( !SetupData.bLocalPossessionAndBeginPlay_Complete && IsLocallyControlled() && HasActorBegunPlay() )
	{
		Server_OnLocalCharacterPossessAndBeginPlay();
		SetupData.bLocalPossessionAndBeginPlay_Complete = true;
	}
}

void AShireCharacter::CheckFor_ClientLocalPlayerStateAndPlayerState()
{
	if ( !SetupData.bClientLocalPlayerStateAndPlayerState_Complete && GetPlayerState() && ClientLocalPlayer && ClientLocalPlayer->PlayerState )
	{
		OnAcknowledgePlayerState();
		SetupData.bClientLocalPlayerStateAndPlayerState_Complete = true;
	}
}

void AShireCharacter::SetCameraPerspective(bool bFirstPerson)
{
	// WARNING - don't get components by index as their order might change.
	USkeletalMeshComponent* rifle3P = Cast<USkeletalMeshComponent>(MeshTP->GetChildComponent(0));

	if (bFirstPerson && !FirstPersonCameraComponent->IsActive())
	{
		FirstPersonCameraComponent->SetActive(true);
		ThirdPersonCameraComponent->SetActive(false);

		Mesh1P->SetVisibility(true, true);
		MeshTP->SetOwnerNoSee(true);

		if (rifle3P)
		{
			rifle3P->SetOwnerNoSee(true);
		}
	}
	else if (!bFirstPerson && !ThirdPersonCameraComponent->IsActive())
	{
		FirstPersonCameraComponent->SetActive(false);
		ThirdPersonCameraComponent->SetActive(true);

		Mesh1P->SetVisibility(false, true);
		MeshTP->SetOwnerNoSee(false);

		if (rifle3P)
		{
			rifle3P->SetOwnerNoSee(false);
		}
	}
}

void AShireCharacter::ToggleCameraPerspective()
{
	if (FirstPersonCameraComponent->IsActive())
	{
		SetCameraPerspective(false);
	}
	else
	{
		SetCameraPerspective(true);
	}
}

void AShireCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShireCharacter, Team)
	DOREPLIFETIME(AShireCharacter, Health)
	DOREPLIFETIME(AShireCharacter, SecondaryStateData)
	DOREPLIFETIME(AShireCharacter, LastImpactDirection)

}

void AShireCharacter::OnLocalPlayerAcknowledgePossession(AShirePlayerController* TargetPlayerController)
{
	// Triggered by local players only.
	LocalShirePlayerController = TargetPlayerController;
}

void AShireCharacter::OnHealthUpdate()
{
	OnHealthChange.Broadcast(); // Broadcast to non local players as well to update TP HealthBar.
	if (IsDead()) 
	{ 
		Die(); 
	}
}

void AShireCharacter::Server_OnLocalCharacterPossessAndBeginPlay_Implementation()
{
	AShireGameMode* shireGameMode = Cast<AShireGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	shireGameMode->OnLocalCharacterPossessAndBeginPlay();
}

void AShireCharacter::OnRep_Health()
{
	OnHealthUpdate();
}

void AShireCharacter::SetHealth(float HealthValue)
{
	// Only the server should actually change health. Then it will replicate to clients.
	if (GetLocalRole() == ROLE_Authority)
	{
		Health = FMath::Clamp(HealthValue, 0.f, MaxHealth);
		OnHealthUpdate();
	}
}

bool AShireCharacter::IsDead() const
{
	return (Health <= 0);
}

bool AShireCharacter::IsTeammate(AShireCharacter* OtherPlayer) const
{
	return GetTeam() == OtherPlayer->GetTeam();
}

int AShireCharacter::GetTeam() const
{
	return Team;
}

void AShireCharacter::PossessedBy(AController* NewController)
{
	// Runs on Server only.
	Super::PossessedBy(NewController);
	
	Multicast_OnPossessed();
	
	AShireAIController* botController = Cast<AShireAIController>(NewController);
	if ( botController ) { Team = botController->Team; }
}

float AShireCharacter::TakeHealing(float HealingTaken, AController* EventInstigator, AActor* HealingCauser)
{
	float targetHealth = FMath::Clamp(Health + HealingTaken, 0.f, MaxHealth);
	float healingApplied = targetHealth - Health;
	SetHealth(targetHealth);
	OnTakeHealing.Broadcast(GetController(), EventInstigator, healingApplied);
	return healingApplied;
}

float AShireCharacter::TakeDamage(float DamageTaken, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (IsDead()) { return 0; }
	UAISense_Damage::ReportDamageEvent(GetWorld(), this, DamageCauser, DamageTaken, DamageCauser->GetActorLocation(), GetActorLocation());

	float targetHealth = FMath::Clamp(Health - DamageTaken, 0.f, MaxHealth);
	float damageApplied = Health - targetHealth;
	SetHealth(targetHealth);
	OnTakeDamage.Broadcast(GetController(), EventInstigator, damageApplied);
	return damageApplied;
}

void AShireCharacter::SetLastImpactDirection(FVector ImpactDirection) 
{
	// A unit vector pointing in the direction of the most recent hit impact.
	LastImpactDirection = ImpactDirection;
}

void AShireCharacter::Die()
{
	Health = 0; // For when forcing a manual death. Set Health to 0 so IsDead() returns true.
	ToggleScopeView(false);
	MovementComponent->DisableMovement();
	MeshTP->bPauseAnims = true;
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MeshTP->SetAllBodiesSimulatePhysics(true);

	if (!LastImpactDirection.IsZero())
	{
		FVector targetImpulse = LastImpactDirection * DeathImpulseScalar;
		MeshTP->AddImpulse(targetImpulse, "Spine_03", true);

		FDetachmentTransformRules detachmentRules = FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true);
		RangedWeaponTP->DetachFromComponent(detachmentRules);
		RangedWeaponTP->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
		RangedWeaponTP->SetSimulatePhysics(true);
		MeleeWeaponTP->DetachFromComponent(detachmentRules);
		MeleeWeaponTP->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
		MeleeWeaponTP->SetSimulatePhysics(true);
	}

	SetCameraPerspective(false);
	OnDeath.Broadcast(this);

}

void AShireCharacter::SetupCharacterMesh_Implementation()
{
	// Defined in BP.
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MOVEMENT
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AShireCharacter::Multicast_SlideCrouch_Implementation()
{
	float offsetZ = BaseCapsuleHalfHeight - MovementComponent->CrouchedHalfHeight;
	FVector offset = FVector(0, 0, offsetZ);
	GetCapsuleComponent()->SetCapsuleHalfHeight(MovementComponent->CrouchedHalfHeight);
	GetCapsuleComponent()->AddLocalOffset(-offset);
	FirstPersonCameraComponent->AddLocalOffset(offset);
	BaseCameraLocation = BaseCameraLocation + offset;
	MeshTP->AddLocalOffset(offset);
	
	if (!IsLocallyControlled())
	{
		CacheInitialMeshOffset(MeshTP->GetRelativeLocation(), MeshTP->GetRelativeRotation());
	}

}

void AShireCharacter::Server_SlideCrouch_Implementation()
{
	Multicast_SlideCrouch();
}

void AShireCharacter::SlideCrouch()
{
	Server_SlideCrouch();
}

void AShireCharacter::SlideUnCrouch()
{
	float offsetZ = MovementComponent->CrouchedHalfHeight - BaseCapsuleHalfHeight;
	FVector offset = FVector(0, 0, offsetZ);
	GetCapsuleComponent()->SetCapsuleHalfHeight(BaseCapsuleHalfHeight);
	GetCapsuleComponent()->AddLocalOffset(-offset);
	FirstPersonCameraComponent->AddLocalOffset(offset);
	BaseCameraLocation = BaseCameraLocation + offset;
	MeshTP->AddLocalOffset(offset);

	if (!IsLocallyControlled())
	{
		CacheInitialMeshOffset(FVector(BaseMeshLocation), MeshTP->GetRelativeRotation());
	}

}

void AShireCharacter::Server_UseSlideAbility_Implementation()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (MovementComponent->IsMovingOnGround())
		{
			SecondaryStateData.bIsSliding = true;

			// Slide physics
			SlideDirection = GetActorForwardVector();
			FVector slideVelocity = SlideDirection * SlideHorizontalSpeed;
			MovementComponent->GroundFriction = 0;
			MovementComponent->Velocity = slideVelocity;
		}
	}
}

void AShireCharacter::Multicast_CancelSlide_Implementation()
{
	if (SecondaryStateData.bIsSliding)
	{		
		SecondaryStateData.bIsSliding = false;
		MovementComponent->GroundFriction = BaseGroundFriction;
		SlideUnCrouch();
	}
}

void AShireCharacter::Server_CancelSlide_Implementation()
{
	Multicast_CancelSlide();
}

void AShireCharacter::CancelSlide()
{
	Server_CancelSlide();
}

void AShireCharacter::CheckFor_SlideCancel(float DeltaTime)
{
	if ( SecondaryStateData.bIsSliding && GetLocalRole() == ROLE_Authority ) // Only server should cancel slide since it multicasts.
	{

		float currentSpeed = UKismetMathLibrary::VSizeXY(MovementComponent->Velocity);
		if (currentSpeed <= 10)
		{
			CancelSlide();
		}
		else if (MovementComponent->IsFalling())
		{
			float targetX = MovementComponent->Velocity.X - (DeltaTime * SlideDecelerationRate * MovementComponent->Velocity.X);
			float targetY = MovementComponent->Velocity.Y - (DeltaTime * SlideDecelerationRate * MovementComponent->Velocity.Y);
			FVector targetVelocity = FVector(targetX, targetY, MovementComponent->Velocity.Z);
			MovementComponent->Velocity = targetVelocity;
		}
	}
}

void AShireCharacter::SetMaxWalkSpeed(float NewMaxWalkSpeed)
{
	MovementComponent->MaxWalkSpeed = NewMaxWalkSpeed;
	Server_SetMaxWalkSpeed(NewMaxWalkSpeed);
}

void AShireCharacter::Server_SetMaxWalkSpeed_Implementation(float NewMaxWalkSpeed)
{
	MovementComponent->MaxWalkSpeed = NewMaxWalkSpeed;
}

bool AShireCharacter::Server_SetMaxWalkSpeed_Validate(float NewMaxWalkSpeed)
{
	if (NewMaxWalkSpeed > WalkSpeed * 2)
	{
		return false; // Disconnect the caller
	}
	return true; // Allow the RPC call
}

void AShireCharacter::Jump()
{
	Super::Jump();
	WallJump();
	SlideJump();
}

void AShireCharacter::WallJump()
{
	if (MovementComponent->IsFalling() && WallJumpCooldownData.bEnabled)
	{
		FVector rayStart = GetActorLocation();
		FVector rayEnd = GetActorLocation() + (GetActorForwardVector() * WallJumpScanDistance);

		FCollisionQueryParams traceParams(SCENE_QUERY_STAT(WallJump), true, GetInstigator());
		FHitResult hit(ForceInit);
		GetWorld()->LineTraceSingleByChannel(hit, rayStart, rayEnd, ECC_Visibility, traceParams);

		// Can probably just check if (hit)? Most likely want to filter walls by visibility channel to avoid this altogether.
		AActor* hitActor = Cast<AActor>(hit.GetActor());
		if (hitActor)
		{
			MovementComponent->Velocity = FVector::Zero();
			FVector direction = UKismetMathLibrary::GetDirectionUnitVector(hit.ImpactPoint, hit.ImpactPoint + hit.ImpactNormal);
			FVector launchVelocity = (direction * WallJumpHorizontalSpeed) + FVector(0, 0, WallJumpVerticalVelocity);
			LaunchCharacter(launchVelocity, true, true);
			SecondaryStateData.bIsWallJumping = true;
			EnableWallJumpAbility(false);
			OnWallJump.Broadcast();
		}
	}
}

void AShireCharacter::EnableWallJumpAbility(bool bEnable)
{
	WallJumpCooldownData.bEnabled = bEnable;
}

void AShireCharacter::CheckFor_WallJumpReset()
{
	if (!MovementComponent->IsFalling() && !WallJumpCooldownData.bEnabled)
	{
		EnableWallJumpAbility(true);
	}
}

void AShireCharacter::UseSlideAbility()
{
	if (MovementComponent->IsMovingOnGround())
	{
		// Launch on clients to keep velocity in-sync with server.
		if (SlideCooldownData.bEnabled && GetLocalRole() != ROLE_Authority) 
		{
			SecondaryStateData.bIsSliding = true;
			SlideDirection = GetActorForwardVector();
			FVector slideVelocity = SlideDirection * SlideHorizontalSpeed;
			MovementComponent->GroundFriction = 0;
			MovementComponent->Velocity = slideVelocity;
		}

		if (SlideCooldownData.bEnabled)
		{
			Server_UseSlideAbility();
			SlideCrouch();
			EnableSlideAbility(false);

			FTimerDelegate enableSlideAbility;
			enableSlideAbility.BindUFunction(this, FName("EnableSlideAbility"), true);
			GetWorldTimerManager().SetTimer(SlideCooldownData.Timer, enableSlideAbility, SlideCooldownData.Duration, false, SlideCooldownData.Duration);

			OnSlide.Broadcast();
		}
	}
}

void AShireCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	OnLand.Broadcast();
}

void AShireCharacter::EnableSlideAbility(bool bEnable)
{
	SlideCooldownData.bEnabled = bEnable;
}

void AShireCharacter::SlideJump()
{
	float currentSpeed = UKismetMathLibrary::VSizeXY(MovementComponent->Velocity);

	if (currentSpeed >= 10 && SecondaryStateData.bIsSliding)
	{
		FVector launchVelocity = SlideDirection * SlideJumpHorizontalSpeed;
		launchVelocity.Z = SlideJumpVerticalVelocity;
		LaunchCharacter(launchVelocity, true, true);
		OnSlideJump.Broadcast();
	}

	if (GetLocalRole() == ROLE_Authority) // Only Server should cancel slide since multicasts.
	{
		CancelSlide(); // Cancel slide whether the slide jump was successful or not.
	}
}

void AShireCharacter::UpdateSlideCamera(float DeltaTime)
{
	float sourceCameraLocationZ = FirstPersonCameraComponent->GetRelativeLocation().Z;
	float targetCameraLocationZ;

	bool bSwitched = (SecondaryStateData.bIsSliding != SecondaryStateData.bWasSliding);

	if (SecondaryStateData.bIsSliding)
	{
		SecondaryStateData.bWasSliding = true;
		targetCameraLocationZ = BaseCameraLocation.Z - SlideCameraOffsetZ;
	}
	else
	{
		SecondaryStateData.bWasSliding = false;
		targetCameraLocationZ = BaseCameraLocation.Z;
	}

	float currentCameraLocationZ = FirstPersonCameraComponent->GetRelativeLocation().Z;
	if (bSwitched)
	{
		SlideCameraLerpAlpha = 0; // Reset alpha if we change lerp directions.
	}

	// Lerp
	FVector newCameraLocation = BaseCameraLocation;
	if (currentCameraLocationZ != targetCameraLocationZ)
	{
		SlideCameraLerpAlpha = SlideCameraLerpAlpha + (SlideCameraLerpSpeed * DeltaTime);
		SlideCameraLerpAlpha = std::max(0.0f, std::min(SlideCameraLerpAlpha, 1.0f)); // Clamp
		newCameraLocation.Z = UKismetMathLibrary::Lerp(sourceCameraLocationZ, targetCameraLocationZ, SlideCameraLerpAlpha);
		FirstPersonCameraComponent->SetRelativeLocation(newCameraLocation);
	}

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMBAT
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AShireCharacter::CheckAimAttempt()
{
	if (SecondaryStateData.bAttemptingAim) {
		AimRangedWeapon();
	}
}

void AShireCharacter::AimRangedWeapon()
{
	SecondaryStateData.bAttemptingAim = true;

	if (EquippedWeapon->GetClass() == RangedWeapon->GetClass()
		&& StateManager->ChangeState(EState::AIM))
	{
		OnAim.Broadcast();
	}
}

void AShireCharacter::RelaxRangedWeapon()
{
	SecondaryStateData.bAttemptingAim = false;

	if (EquippedWeapon->GetClass() == RangedWeapon->GetClass()
		&& StateManager->GetCurrentStateName() == EState::AIM
			&& StateManager->ChangeState(EState::IDLE))
	{
		OnAimRelease.Broadcast();
	}
}

void AShireCharacter::ToggleScopeView(bool bShowScope)
{
	if (bShowScope && FirstPersonCameraComponent->IsActive())
	{
		FirstPersonCameraComponent->SetFieldOfView(ScopeFOV);
		RangedWeapon->SetActorHiddenInGame(true);

		if (ScopeOverlay)
		{
			ScopeOverlay->SetVisibility(ESlateVisibility::Visible);
		}
	}
	else
	{
		FirstPersonCameraComponent->SetFieldOfView(90);
		RangedWeapon->SetActorHiddenInGame(false);

		if (ScopeOverlay)
		{
			ScopeOverlay->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AShireCharacter::UseEquippedWeapon()
{
	bool bInValidState = (StateManager->GetCurrentStateName() == EState::AIM || StateManager->GetCurrentStateName() == EState::IDLE);
	if ( EquippedWeapon && bInValidState && !IsDead() ) 
	{
		EquippedWeapon->AttemptActivate();
	}
}

void AShireCharacter::Multicast_SwitchEquippedWeapon_Implementation()
{
	if (EquippedWeapon == RangedWeapon)
	{
		EquippedWeapon = MeleeWeapon;
		MeleeWeaponTP->bCastHiddenShadow = true;
		RangedWeaponTP->bCastHiddenShadow = false;
	}
	else
	{
		EquippedWeapon = RangedWeapon;
		MeleeWeaponTP->bCastHiddenShadow = false;
		RangedWeaponTP->bCastHiddenShadow = true;
	}
};

void AShireCharacter::Server_SwitchEquippedWeapon_Implementation()
{
	Multicast_SwitchEquippedWeapon();
};

void AShireCharacter::SwitchEquippedWeapon() 
{
	if (SecondaryStateData.bCanSwitchWeapon)
	{
		if (StateManager->ChangeState(EState::EQUIP))
		{
			SecondaryStateData.bCanSwitchWeapon = false;
			Server_SwitchEquippedWeapon();
			OnSwitchWeapon.Broadcast();
		}
	}
};

bool AShireCharacter::CanSwitchWeapon() const
{
	if (SecondaryStateData.bCanSwitchWeapon)
	{
		if (StateManager->AllowsNextState(EState::EQUIP))
		{
			return true;
		}
		
	}
	
	return false;

};

void AShireCharacter::EnableWeaponSwitch()
{
	StateManager->ChangeState(EState::IDLE);
	CheckAimAttempt();
	SecondaryStateData.bCanSwitchWeapon = true;
}

FTransform AShireCharacter::GetRangedWeaponSocket(FName SocketName) const
{
	if (RangedWeaponTP)
	{
		return RangedWeaponTP->GetSocketTransform(SocketName);
	}
	
	return FTransform();
};

void AShireCharacter::OnMeleeComplete()
{
	StateManager->ChangeState(EState::IDLE);
};

void AShireCharacter::OnFireComplete()
{
	StateManager->ChangeState(EState::IDLE);
	CheckAimAttempt();
};

void AShireCharacter::Server_RegisterHit_Implementation(AShireCharacter* HitPlayer, float Damage, float Healing, const FVector &ImpactDirection)
{
	if (IsTeammate(HitPlayer))
	{
		HitPlayer->TakeHealing(Healing, GetController(), this);
	}
	else
	{
		HitPlayer->SetLastImpactDirection(ImpactDirection); // Cache the impact direction unit vector in case hit player dies and we impulse ragdoll.
		HitPlayer->TakeDamage(Damage, FDamageEvent(), GetController(), this);
	}
}

void AShireCharacter::SpawnProjectileTrail(TSubclassOf<AActor> ProjectileTrailClass, const FVector &PlayerViewLocation, const FRotator &PlayerViewRotation, float StartDistance)
{
	FVector muzzleLocation;

	if (IsLocallyControlled() && IsPlayerControlled()) // Spawn from FP muzzle.
	{
		muzzleLocation = RangedWeapon->MeshComponent->GetSocketLocation("BarrelSocket");
	}
	else // Spawn from TP muzzle.
	{
		muzzleLocation = RangedWeaponTP->GetSocketLocation("BarrelSocket");
	}
	
	FVector startLocation = muzzleLocation + (PlayerViewRotation.Vector() * StartDistance);
	FActorSpawnParameters spawnParams;
	GetWorld()->SpawnActor<AActor>(ProjectileTrailClass, startLocation, PlayerViewRotation, spawnParams);
}

void AShireCharacter::Server_SpawnProjectileTrail_Implementation(TSubclassOf<AActor> ProjectileTrailClass, const FVector &PlayerViewLocation, const FRotator &PlayerViewRotation, float StartDistance)
{
	Multicast_SpawnProjectileTrail(ProjectileTrailClass, PlayerViewLocation, PlayerViewRotation, StartDistance);
}

void AShireCharacter::Multicast_SpawnProjectileTrail_Implementation(TSubclassOf<AActor> ProjectileTrailClass, const FVector &PlayerViewLocation, const FRotator &PlayerViewRotation, float StartDistance)
{
	if (!IsLocallyControlled())  // Ignore RPC if local player since spawned trail already. WAIT this might be an issue since AI considered locally controlled? AI looks correct tho...
	{
		SpawnProjectileTrail(ProjectileTrailClass, PlayerViewLocation, PlayerViewRotation, StartDistance);
	}
}

void AShireCharacter::ApplyWeaponAimEffects(float AimPercent)
{
	if (!IsLocallyControlled()) { return; }
	ADSPercent = AimPercent;

	if (ADSPercent > ToggleScopePercentThreshold && !SecondaryStateData.bUsingScopeView)
	{
		if (LocalShirePlayerController) // Only care to apply to locally controlled Characters.
		{
			LocalShirePlayerController->ToggleLookSensitivity(true);
			ToggleScopeView(true);
		}
		
		SetMaxWalkSpeed(WalkSpeed * ADSMovementScalar);
		SecondaryStateData.bUsingScopeView = true;
		
	}
	else if (ADSPercent < ToggleScopePercentThreshold && SecondaryStateData.bUsingScopeView)
	{
		if (LocalShirePlayerController) // Only care to apply to locally controlled Characters.
		{
			LocalShirePlayerController->ToggleLookSensitivity(false);
			ToggleScopeView(false);
		}
		
		SetMaxWalkSpeed(WalkSpeed);
		SecondaryStateData.bUsingScopeView = false;

	}
}

void AShireCharacter::ApplyWeaponRecoil(float LerpAlpha)
{
	if (!RangedWeapon || !IsLocallyControlled()) { return; }
	
	// NOTE - Should use PlayerController->SetControlRotation(newRotation) instead of AddControllerPitchInput.
	float inputPitchScale = ((APlayerController*)GetController())->GetDeprecatedInputPitchScale();
	//float InputYawScale = PlayerController->GetDeprecatedInputYawScale();
	//Pawn->AddControllerYawInput(HorizontalRecoilStrength / InputYawScale);

	float recoilStrength = RangedWeapon->RecoilData.KickStrength;
	if (LerpAlpha == 1)
	{
		AddControllerPitchInput(recoilStrength / inputPitchScale * LerpAlpha);
	}
	else
	{
		AddControllerPitchInput(recoilStrength / inputPitchScale * LerpAlpha * -1);
	}
}

void AShireCharacter::OnMeleeWeaponActivate()
{
	StateManager->ChangeState(EState::MELEE);
}

void AShireCharacter::OnRangedWeaponActivate()
{
	if (StateManager->GetCurrentStateName() == EState::AIM)
	{
		ToggleScopeView(false);	
	}

	StateManager->ChangeState(EState::FIRE);
	Server_OnRangedWeaponActivate();

}

void AShireCharacter::Server_OnRangedWeaponActivate_Implementation()
{
	Multicast_OnRangedWeaponActivate();
}

void AShireCharacter::Multicast_OnRangedWeaponActivate_Implementation()
{
	OnMuzzleFlash.Broadcast();
}

void AShireCharacter::SetupRecoilTimeline()
{
	if (!RangedWeapon) { return; }

	float recoilRecoveryTime = RangedWeapon->RecoilData.RecoveryDuration;
	float recoilKickTime = RangedWeapon->RecoilData.KickDuration;

	RecoilTimelineCurve = NewObject<UCurveFloat>();
	RecoilTimelineCurve->FloatCurve.AddKey(0, 1);
	RecoilTimelineCurve->FloatCurve.AddKey(recoilKickTime, 1);

	// Maintaining same area under the curve for recovery period.
	float recoveryValue = recoilKickTime / recoilRecoveryTime;
	if (recoveryValue == 1)
	{
		recoveryValue = 0.99;
	}

	RecoilTimelineCurve->FloatCurve.AddKey(recoilKickTime + 0.0001, recoveryValue);
	RecoilTimelineCurve->FloatCurve.AddKey(recoilKickTime + recoilRecoveryTime, recoveryValue);

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UI
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AShireCharacter::SetupWidgets(UShireHUDWidget* TargetHUD, UUserWidget* TargetScopeOverlay)
{
	if ( !HUD )
	{
		HUD = TargetHUD;
		HUD->SetCharacter(this);
		OnHUDReady.Broadcast();
		ScopeOverlay = TargetScopeOverlay;
		ToggleScopeView(false);
	}
}

void AShireCharacter::SetupHealthBar()
{
	if ( TPHealthBarComponent && ClientLocalPlayer )
	{
		UTPHealthBarWidget* healthBarWidget = Cast<UTPHealthBarWidget>(TPHealthBarComponent->GetUserWidgetObject());

		if ( healthBarWidget )
		{
			// Set this Character as widget owner.
			healthBarWidget->BindToCharacterEvents(this);

			// Set widget color based on this client's local player team.
			AShireCharacter* localCharacter = Cast<AShireCharacter>(ClientLocalPlayer->GetPawn());
			healthBarWidget->SetColor(IsTeammate(localCharacter));
		}
	}
}

void AShireCharacter::SetupNonLocalThirdPerson()
{
	SetupHealthBar();
	SetupCharacterMesh();
}

void AShireCharacter::CheckFor_NonLocalThirdPersonSetup()
{
	if ( !SetupData.bNonLocalThirdPersonSetup_Complete && ClientLocalPlayer && ClientLocalPlayer->GetPawn() )
	{
		if ( TPHealthBarComponent && TPHealthBarComponent->HasBegunPlay() )
		{
			AShireCharacter* localCharacter = Cast<AShireCharacter>(ClientLocalPlayer->GetPawn());
			if ( GetTeam() != 0 && localCharacter && localCharacter->GetTeam() != 0 )
			{
				SetupNonLocalThirdPerson();
				SetupData.bNonLocalThirdPersonSetup_Complete = true;
			}
		}
	}
}

void AShireCharacter::UpdateHealthBarRotation()
{
	if ( !(IsLocallyControlled() && IsPlayerControlled()) && TPHealthBarComponent )
	{
		AShireGameState* gameState = Cast<AShireGameState>(UGameplayStatics::GetGameState(GetWorld()));
		if (gameState->LocalPlayer)
		{
			FVector localPlayerCameraLocation = gameState->LocalPlayer->PlayerCameraManager->GetCameraLocation();
			FRotator targetRotation = UKismetMathLibrary::FindLookAtRotation(TPHealthBarComponent->GetComponentLocation(), localPlayerCameraLocation);
			TPHealthBarComponent->SetWorldRotation(targetRotation);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TEMPLATE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AShireCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Keyboard::UseWeapon", IE_Pressed, this, &AShireCharacter::UseEquippedWeapon);
	PlayerInputComponent->BindAction("Keyboard::Aim", IE_Pressed, this, &AShireCharacter::AimRangedWeapon);
	PlayerInputComponent->BindAction("Keyboard::Aim", IE_Released, this, &AShireCharacter::RelaxRangedWeapon);
	PlayerInputComponent->BindAction("Keyboard::Jump", IE_Pressed, this, &AShireCharacter::Jump);
	PlayerInputComponent->BindAction("Keyboard::Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Keyboard::Slide", IE_Pressed, this, &AShireCharacter::UseSlideAbility);
	PlayerInputComponent->BindAction("Keyboard::SwapWeapon", IE_Pressed, this, &AShireCharacter::SwitchEquippedWeapon);

	PlayerInputComponent->BindAction("Controller::UseWeapon", IE_Pressed, this, &AShireCharacter::UseEquippedWeapon);
	PlayerInputComponent->BindAction("Controller::Aim", IE_Pressed, this, &AShireCharacter::AimRangedWeapon);
	PlayerInputComponent->BindAction("Controller::Aim", IE_Released, this, &AShireCharacter::RelaxRangedWeapon);
	PlayerInputComponent->BindAction("Controller::Jump", IE_Pressed, this, &AShireCharacter::Jump);
	PlayerInputComponent->BindAction("Controller::Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Controller::Slide", IE_Pressed, this, &AShireCharacter::UseSlideAbility);
	PlayerInputComponent->BindAction("Controller::SwapWeapon", IE_Pressed, this, &AShireCharacter::SwitchEquippedWeapon);

	// Bind movement events
	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &AShireCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &AShireCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "Mouse" versions handle devices that provide an absolute delta, such as a mouse.
	// "Gamepad" versions are for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &AShireCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &AShireCharacter::LookUpAtRate);
}

FTransform AShireCharacter::GetThirdPersonCameraTransform() const
{
	return ThirdPersonCameraComponent->GetComponentTransform();
}

void AShireCharacter::MoveForward(float Value)
{
	if (Value != 0.0f && !SecondaryStateData.bIsSliding)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AShireCharacter::MoveRight(float Value)
{
	if (Value != 0.0f && !SecondaryStateData.bIsSliding)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AShireCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AShireCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}
