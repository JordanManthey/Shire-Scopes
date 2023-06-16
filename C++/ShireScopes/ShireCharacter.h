// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CooldownData.h"
#include "ShireCharacter.generated.h"

class AWeapon;
class ARangedWeapon;
class AMeleeWeapon;
class AShirePlayerController;
class AShirePlayerState;
class UInputComponent;
class USceneComponent;
class UCameraComponent;
class UCharacterStateManagerComponent;
class UCurveFloat;
class UShireHUDWidget;
class UWidgetComponent;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STRUCT DECLARATIONS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct FShireCharacter_SetupData
{
	bool bLocalPossessionAndBeginPlay_Complete{ false };
	bool bClientLocalPlayerStateAndPlayerState_Complete{ false };
	bool bNonLocalThirdPersonSetup_Complete{ false };
};

USTRUCT(BlueprintType)
struct FShireCharacter_SecondaryStateData
{
	GENERATED_USTRUCT_BODY()

	bool bCanSwitchWeapon{ true };
	bool bUsingScopeView{ false };
	bool bAttemptingAim{ false };
	bool bWasSliding{ false };

	UPROPERTY(BlueprintReadWrite)
	bool bIsWallJumping{ false };

	UPROPERTY(BlueprintReadOnly)
	bool bIsSliding{ false };

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DELEGATE DECLARATIONS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHUDReady);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHealthChange);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSlideJump);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWallJump);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLand);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSlide);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSwitchWeapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAim);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAimRelease);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMuzzleFlash);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AShireCharacter*, DeathCharacter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTakeDamage, AController*, DamageReceiver, AController*, DamageDealer, float, DamageAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTakeHealing, AController*, HealingReceiver, AController*, HealingDealer, float, HealingAmount);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GENERAL
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * The main Character class that is to be possessed by all players/bots in all playable GameModes.
 * Refer to BP_ShireCharacter for additional functionality.
 */
UCLASS(config=Game)
class AShireCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	
	AShireCharacter();

	///////////////////////////////////////////////////////////////////////////////////
	// EVENTS
	///////////////////////////////////////////////////////////////////////////////////

	/** Triggered when all HUD widgets are setup. */
	UPROPERTY(BlueprintAssignable)
	FOnHUDReady OnHUDReady;

	/** Triggered when this character begins to aim its ranged weapon. */
	UPROPERTY(BlueprintAssignable)
	FOnAim OnAim;

	/** Triggered when this character releases aim on its ranged weapon. */
	UPROPERTY(BlueprintAssignable)
	FOnAimRelease OnAimRelease;

	/** Triggered when the muzzle flash is activated on this character's ranged weapon. */
	UPROPERTY(BlueprintAssignable)
	FOnMuzzleFlash OnMuzzleFlash;

	/** Triggered when this character dies. */
	UPROPERTY(BlueprintAssignable)
	FOnDeath OnDeath;

	/** Triggered when this character's health changes. */
	UPROPERTY(BlueprintAssignable)
	FOnHealthChange OnHealthChange;

	/** Triggered when this character takes damage. */
	UPROPERTY(BlueprintAssignable)
	FOnTakeDamage OnTakeDamage;

	/** Triggered when this character takes healing. */
	UPROPERTY(BlueprintAssignable)
	FOnTakeDamage OnTakeHealing;

	///////////////////////////////////////////////////////////////////////////////////
	// DATA
	///////////////////////////////////////////////////////////////////////////////////

	/** The struct used for managing this character's secondary states, which are not mutually exclusive like the primary states. */
	UPROPERTY(BlueprintReadWrite, Replicated)
	FShireCharacter_SecondaryStateData SecondaryStateData;

	///////////////////////////////////////////////////////////////////////////////////
	// VARIABLES
	///////////////////////////////////////////////////////////////////////////////////

	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly)
	float Health;

	/** The locally controlled player on this client. Useful for setting up nonlocal characters. */
	UPROPERTY(BlueprintReadOnly)
	APlayerController* ClientLocalPlayer;

	/** This character's controller which is only set if is locally controlled. */
	AShirePlayerController* LocalShirePlayerController;

	///////////////////////////////////////////////////////////////////////////////////
	// FUNCTIONS
	///////////////////////////////////////////////////////////////////////////////////

	/** @returns the primary state of this character. */
	UFUNCTION(BlueprintCallable)
	EState GetPlayerStateName() const;

	/** @returns true if this character is on the same team as @param OtherPlayer. */
	UFUNCTION(BlueprintCallable)
	bool IsTeammate(AShireCharacter* OtherPlayer) const;

	/** @returns the team of this character. May not be set until team data has been passed by Lobby. */
	UFUNCTION(BlueprintCallable)
	int GetTeam() const;

	/** @returns true if this character is dead. */
	UFUNCTION(BlueprintCallable)
	bool IsDead() const;

	void PossessedBy(AController* NewController) override;
	float TakeHealing(float HealingTaken, AController* EventInstigator, AActor* HealingCauser);
	float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	/**
	 * Caches the direction of impact of a damage-taking hit on this character. 
	 * Used for determining the ragdoll fall direction of this character upon death.
	 * @param ImpactDirection the direction unit vector of the hit.
	 */
	void SetLastImpactDirection(FVector ImpactDirection);

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Triggered when a local controller possesses this character. */
	void OnLocalPlayerAcknowledgePossession(AShirePlayerController* TargetPlayerController);

protected:

	///////////////////////////////////////////////////////////////////////////////////
	// VARIABLES
	///////////////////////////////////////////////////////////////////////////////////

	UPROPERTY(Replicated)
	int Team{ 0 };

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "General")
	float MaxHealth{ 100 };

	/** The force applied in the LastImpactDirection to push the ragdoll upon death. */
	UPROPERTY(EditDefaultsOnly, Category = "General")
	float DeathImpulseScalar{ 5000 };

	/** The direction of impact of the most recent damage-taking hit on this character. */
	UPROPERTY(Replicated) 
	FVector LastImpactDirection; // Worried this might not work over real multiplayer where local death function might be called before LastImpactDirection is replicated.

	///////////////////////////////////////////////////////////////////////////////////
	// FUNCTIONS
	///////////////////////////////////////////////////////////////////////////////////

	/** Triggered on the Server when a character has begun play and has been possessed by a local controller. */
	UFUNCTION(Server, Reliable)
	void Server_OnLocalCharacterPossessAndBeginPlay();

	/** Sets up both the first person and third person meshes for this character. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetupCharacterMesh();

	/** Triggered on all clients when the replicated Health member is updated. */
	UFUNCTION()
	void OnRep_Health();

	/** Triggered on all clients when the replicated Health member is updated. */
	void OnHealthUpdate();

	/** Toggles the active camera between first person and third person. For Debug purposes only. */
	UFUNCTION(BlueprintCallable)
	void ToggleCameraPerspective();

	/** Sets the active camera to first or third person based on @param bFirstPerson. For Debug purposes only. */
	void SetCameraPerspective(bool bFirstPerson);

	/** Kills this character and handles its death behavior. */
	UFUNCTION(BlueprintCallable)
	void Die();

	virtual void BeginPlay();
	virtual void Tick(float DeltaTime) override;
	void SetHealth(float healthValue);

private:

	///////////////////////////////////////////////////////////////////////////////////
	// DATA
	///////////////////////////////////////////////////////////////////////////////////

	/** The struct used to flag various setup processes as completed. Useful for tracking character setup and enabling tick queries. */
	FShireCharacter_SetupData SetupData;

	///////////////////////////////////////////////////////////////////////////////////
	// VARIABLES
	///////////////////////////////////////////////////////////////////////////////////

	/** The component used for managing the primary states (mutually exclusive) for this character. */
	UCharacterStateManagerComponent* StateManager;

	///////////////////////////////////////////////////////////////////////////////////
	// FUNCTIONS
	///////////////////////////////////////////////////////////////////////////////////

	/** Notifies all client instances of this character when the Server has been possessed by a controller. */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnPossessed();

	/** Used for making viewport components that are setup in BP_ShireCharacter readily accessible here in C++. */
	void SetupComponentReferences();

	/** Triggered when this character's PlayerState becomes available. */
	void OnAcknowledgePlayerState();

	void CheckFor_GameStateLocalPlayer();
	void CheckFor_ClientLocalPlayerStateAndPlayerState();
	void CheckFor_LocalPossessionAndBeginPlay();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MOVEMENT
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

public:

	///////////////////////////////////////////////////////////////////////////////////
	// EVENTS
	///////////////////////////////////////////////////////////////////////////////////

	/** Triggered when this character uses its slide ability. */
	UPROPERTY(BlueprintAssignable)
	FOnSlide OnSlide;

	/** Triggered when this character uses its slide jump ability. */
	UPROPERTY(BlueprintAssignable)
	FOnSlideJump OnSlideJump;

	/** Triggered when this character uses its wall jump ability. */
	UPROPERTY(BlueprintAssignable)
	FOnWallJump OnWallJump;

	/** Triggered when this character lands after a jump. */
	UPROPERTY(BlueprintAssignable)
	FOnLand OnLand;

	///////////////////////////////////////////////////////////////////////////////////
	// FUNCTIONS
	///////////////////////////////////////////////////////////////////////////////////

	/** Attempts to perform a slide if character requirements are met. */
	UFUNCTION(BlueprintCallable)
	void UseSlideAbility();

	virtual void Landed(const FHitResult& Hit) override;

protected:

	///////////////////////////////////////////////////////////////////////////////////
	// DATA
	///////////////////////////////////////////////////////////////////////////////////

	/** The struct used for handle wall jump cooldowns. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jump")
	FCooldownData WallJumpCooldownData;

	/** The struct used for handle slide cooldowns. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Slide")
	FCooldownData SlideCooldownData;

	///////////////////////////////////////////////////////////////////////////////////
	// VARIABLES
	///////////////////////////////////////////////////////////////////////////////////

	/** The maximum distance in which a wall can be detected to perform a wall jump. */
	UPROPERTY(EditDefaultsOnly, Category = "Jump")
	float WallJumpScanDistance{ 100 };

	/** The speed at which a wall jump pushes this character in horizontally. */
	UPROPERTY(EditDefaultsOnly, Category = "Jump")
	float WallJumpHorizontalSpeed{ 400 };

	/** The speed at which a wall jump pushes this character in vertically. */
	UPROPERTY(EditDefaultsOnly, Category = "Jump")
	float WallJumpVerticalVelocity{ 600 };

	/** The height offset in which the player's camera should descend till when performing a slide. */
	UPROPERTY(EditDefaultsOnly, Category = "Slide")
	float SlideCameraOffsetZ{ 5 };

	/** The speed in which the camera should move when adjusting for a slide and slide recovery. */
	UPROPERTY(EditDefaultsOnly, Category = "Slide")
	float SlideCameraLerpSpeed{ 1 };

	/** The rate at which this character slows down after performing a slide. */
	UPROPERTY(EditDefaultsOnly, Category = "Slide")
	float SlideDecelerationRate{ 1 };

	/** The initial speed of this character after performing a slide. */
	UPROPERTY(EditDefaultsOnly, Category = "Slide")
	float SlideHorizontalSpeed{ 3250 };

	/** The horizontal launch speed of this character after performing a slide jump. */
	UPROPERTY(EditDefaultsOnly, Category = "Slide")
	float SlideJumpHorizontalSpeed{ 600 };

	/** The vertical launch velocity of this character after performing a slide jump. */
	UPROPERTY(EditDefaultsOnly, Category = "Slide")
	float SlideJumpVerticalVelocity{ 1200 };

	/** The movement speed multiplier for when this character is sprinting. */
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SprintMovementScalar{ 1.50 };

	/** The movement speed multiplier for when this character is aiming its ranged weapon. */
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float ADSMovementScalar{ 0.50 };

	FVector BaseMeshLocation;
	FVector BaseCameraLocation;
	FVector SlideDirection;
	float WalkSpeed;
	float BaseGroundFriction;
	float BaseCapsuleHalfHeight;
	float SlideCameraLerpAlpha{ 0 };

	///////////////////////////////////////////////////////////////////////////////////
	// FUNCTIONS
	///////////////////////////////////////////////////////////////////////////////////

	/** Notifies all clients to be in the slide crouch position. */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SlideCrouch();

	/** Notifies the Server to be in the slide crouch position. */
	UFUNCTION(Server, Reliable)
	void Server_SlideCrouch();

	/** Notifies the Server to attempt to perform a slide. */
	UFUNCTION(Server, Reliable)
	void Server_UseSlideAbility();

	/** Notifies all clients to cancel their current slide. */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_CancelSlide();

	/** Notifies the server to cancel its current slide. */
	UFUNCTION(Server, Reliable)
	void Server_CancelSlide();

	/** Forcefully enables/disables the slide ability for use based on @param bEnable. */
	UFUNCTION()
	void EnableSlideAbility(bool bEnable);

	/** Notifies the server to set its movement speed to @param NewMaxWalkSpeed. */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetMaxWalkSpeed(float NewMaxWalkSpeed);

	void Jump() override;

	/** Attempt to perform a wall jump. */
	void WallJump();

	/** Forcefully enables/disables the wall jump ability for use based on @param bEnable. */
	void EnableWallJumpAbility(bool bEnable);

	/** Puts the character in a crouched position when performing a slide. */
	void SlideCrouch();

	/** Recovers the character from the crouched position after a slide is complete. */
	void SlideUnCrouch();

	/** Cancels the current slide being performed. */
	void CancelSlide();

	/** Attempts to perform a slide jump. */
	void SlideJump();

	/** Tick handling of camera changes during a slide. */
	void UpdateSlideCamera(float DeltaTime);

	void CheckFor_WallJumpReset();
	void CheckFor_SlideCancel(float Deltatime);
	void SetMaxWalkSpeed(float NewMaxWalkSpeed);

private:

	///////////////////////////////////////////////////////////////////////////////////
	// VARIABLES
	///////////////////////////////////////////////////////////////////////////////////

	UCharacterMovementComponent* MovementComponent;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMBAT
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

public:

	///////////////////////////////////////////////////////////////////////////////////
	// VARIABLES
	///////////////////////////////////////////////////////////////////////////////////

	/** Triggered whenever this character successfully switches its weapon. */
	UPROPERTY(BlueprintAssignable)
	FOnSwitchWeapon OnSwitchWeapon;

	/** The time it takes this character to switch its weapon. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon")
	float SwitchWeaponDuration{ 0.5 };

	/** At which point ADSPercent will toggle the scope view for the player. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ToggleScopePercentThreshold{ 0.90 };

	/** The time it takes to full aim this character's ranged weapon. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon")
	float ADSDuration{ 0.3 };

	/** The currently equipped weapon. */
	UPROPERTY(BlueprintReadOnly)
	AWeapon* EquippedWeapon;

	/** The weapon in this character's ranged weapon slot. */
	UPROPERTY(BlueprintReadOnly)
	ARangedWeapon* RangedWeapon;

	/** The weapon in this character's melee weapon slot. */
	UPROPERTY(BlueprintReadOnly)
	AMeleeWeapon* MeleeWeapon;

	/** The current aiming percent of this character. (0.0 = no aim, 1.0 = full aimed) */
	float ADSPercent{ 0 };

	///////////////////////////////////////////////////////////////////////////////////
	// FUNCTIONS
	///////////////////////////////////////////////////////////////////////////////////

	/** 
	 * Notifies the server to register a hit made this character with its equipped weapon. 
	 * @param HitPlayer the player hit by this character.
	 * @param Damage the amount of damage inflicted by this hit.
	 * @param Healing the amount of healing inflicted by this hit.
	 * @param ImpactDirection the direction unit vector of the impact of this hit.
	 */
	UFUNCTION(Server, Reliable)
	void Server_RegisterHit(AShireCharacter* HitPlayer, float Damage, float Healing, const FVector &ImpactDirection);

	/**
	 * Spawn the projectile trail from the third person perspective so other players can see.
	 * @param ProjectileTrailClass the particle actor to be spawned.
	 * @param PlayerViewLocation this character's view location.
	 * @param PlayerViewRotation this character's view rotation.
	 * @param StartDistance the distance from the ranged weapon barrel that the projectile trail should start from.
	 */
	void SpawnProjectileTrail(TSubclassOf<AActor> ProjectileTrailClass, const FVector &PlayerViewLocation, const FRotator &PlayerViewRotation, float StartDistance);
	UFUNCTION(Server, Reliable)
	void Server_SpawnProjectileTrail(TSubclassOf<AActor> ProjectileTrailClass, const FVector &PlayerViewLocation, const FRotator &PlayerViewRotation, float StartDistance);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnProjectileTrail(TSubclassOf<AActor> ProjectileTrailClass, const FVector &PlayerViewLocation, const FRotator &PlayerViewRotation, float StartDistance);

	/** Enable this character to switch weapons. */
	UFUNCTION(BlueprintCallable)
	void EnableWeaponSwitch();

	/** @returns the transform of this character's ranged weapon @param SocketName. */
	UFUNCTION(BlueprintCallable)
	FTransform GetRangedWeaponSocket(FName SocketName) const;

	/** Attempts to activate this character's equipped weapon. */
	UFUNCTION(BlueprintCallable)
	void UseEquippedWeapon();

	/** Attempts to begin aiming this character's ranged weapon. */
	UFUNCTION(BlueprintCallable)
	void AimRangedWeapon();

	/** Unaims this character's ranged weapon. */
	UFUNCTION(BlueprintCallable)
	void RelaxRangedWeapon();

	/** @returns true if this character can switch its weapon. */
	UFUNCTION(BlueprintCallable)
	bool CanSwitchWeapon() const;

protected:

	///////////////////////////////////////////////////////////////////////////////////
	// VARIABLES
	///////////////////////////////////////////////////////////////////////////////////

	/** The FOV of the scoped view when a character has aimed their ranged weapon. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon")
	float ScopeFOV{ 15 };

	/** The timeline curve used when applying camera recoil upon the player. This is generated based on the equipped ranged weapon's recoil attributes. */
	UPROPERTY(BlueprintReadOnly)
	UCurveFloat* RecoilTimelineCurve;

	///////////////////////////////////////////////////////////////////////////////////
	// FUNCTIONS
	///////////////////////////////////////////////////////////////////////////////////

	/** The timeline-tick function used to apply weapon based on the current @param LerpAlpha of the recoil timeline. */
	UFUNCTION(BlueprintCallable)
	void ApplyWeaponRecoil(float LerpAlpha);

	/** Notifies all clients to switch their weapon. */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SwitchEquippedWeapon();

	/** Notifies the server to switch its weapon. */
	UFUNCTION(Server, Reliable)
	void Server_SwitchEquippedWeapon();

private:

	///////////////////////////////////////////////////////////////////////////////////
	// VARIABLES
	///////////////////////////////////////////////////////////////////////////////////

	/** The third person mesh representing this character's ranged weapon. For visuals only. */
	USkeletalMeshComponent* RangedWeaponTP;

	/** The third person mesh representing this character's melee weapon. For visuals only. */
	USkeletalMeshComponent* MeleeWeaponTP;

	///////////////////////////////////////////////////////////////////////////////////
	// FUNCTIONS
	///////////////////////////////////////////////////////////////////////////////////

	/** Triggered when this character successfully uses its melee weapon. */
	UFUNCTION()
	void OnMeleeWeaponActivate();

	/** Triggered when this character successfully uses its ranged weapon. */
	UFUNCTION()
	void OnRangedWeaponActivate();
	UFUNCTION(Server, Reliable)
	void Server_OnRangedWeaponActivate();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnRangedWeaponActivate();

	/** Triggered when this characters melee animation is complete. */
	UFUNCTION(BlueprintCallable)
	void OnMeleeComplete();

	/** Triggered when this characters ranged weapon fire animation is complete. */
	UFUNCTION(BlueprintCallable)
	void OnFireComplete();

	/** Timeline-tick functionality that applies various effects on this character based on its current @param AimPercent. */
	UFUNCTION(BlueprintCallable)
	void ApplyWeaponAimEffects(float AimPercent);

	/** Attempts to switch this character's equpped weaponn. */
	UFUNCTION(BlueprintCallable)
	void SwitchEquippedWeapon();

	/** Used to check if this character is pending to aim its ranged weapon. Used so that a player can hold their input down to apply aim when eventually possible. */
	void CheckAimAttempt(); 

	/** Sets the players view to scope view or normal view based on @param bShowScope. */
	void ToggleScopeView(bool bShowScope);

	/** Generates the recoil timeline based on this character's ranged weapon attributes. */
	void SetupRecoilTimeline();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UI
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

public:

	/** Setups and configures widgets for this character and the controller that possesses it. */
	void SetupWidgets(UShireHUDWidget* TargetHUD, UUserWidget* TargetScopeOverlay);

protected:

	UPROPERTY(BlueprintReadOnly)
	UShireHUDWidget* HUD;	

private:

	UUserWidget* ScopeOverlay;
	UWidgetComponent* TPHealthBarComponent;

	/** Tick functionality that rotates this character's health bar to always face the locally controlled player on this game client. */
	void UpdateHealthBarRotation();

	void SetupHealthBar();
	void SetupNonLocalThirdPerson();
	void CheckFor_NonLocalThirdPersonSetup();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TEMPLATE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

public:

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float TurnRateGamepad;

	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }

	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	FTransform GetThirdPersonCameraTransform() const;

protected:

	/** Handles moving forward/backward */
	void MoveForward(float Val);
	/** Handles strafing movement, left and right */
	void MoveRight(float Val);
	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);
	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

private:

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	USkeletalMeshComponent* MeshTP;
	UCameraComponent* ThirdPersonCameraComponent;
};

