// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterStateManagerComponent.generated.h"

class BaseState; // WARNING: Incomplete class is hiding that its abstract. So BaseState* currentState can error?
class IdleState;
class AimState;
class MeleeState;
class FireState;
class EquipState;

/**
 * Enumerator for each possible primary state.
 */
UENUM(BlueprintType)
enum class EState : uint8 { IDLE UMETA(DisplayName = "IDLE"), AIM UMETA(DisplayName = "AIM"), MELEE UMETA(DisplayName = "MELEE"), EQUIP UMETA(DisplayName = "EQUIP"), FIRE UMETA(DisplayName = "FIRE") };

/**
 * This component manages the primary state machine of every ShireCharacter (AI and Players). Created and attached upon ShireCharacter construction.
 * Replicates the CurrentState and is responsible for state transitions. 
 */

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHIRESCOPES_API UCharacterStateManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCharacterStateManagerComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_CurrentState();
	
	EState GetCurrentStateName() const;
	bool IsInEitherState(EState State1, EState State2) const;
	bool IsInAnyState(const TArray<EState> &States) const;
	bool AllowsNextState(EState NextState) const;
	bool ChangeState(EState State, bool bForceState = false);

protected:

	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentState)
	EState CurrentStateName; // Used for cheap replication

	UFUNCTION(Server, Reliable)
	void Server_SetState(EState State);

private:

	const EquipState* Equip;
	const FireState* Fire;
	const MeleeState* Melee;
	const IdleState* Idle;
	const AimState* Aim;
	const BaseState* CurrentState;

	void SetState(const BaseState* State);
	void ReplicateState(EState State);
	bool ChangeState(const BaseState* State, bool bForceState = false);
	const BaseState* GetStateByName(EState State) const;
		
};
