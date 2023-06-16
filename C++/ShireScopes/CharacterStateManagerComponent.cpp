// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterStateManagerComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "BaseState.h"
#include "IdleState.h"
#include "AimState.h"
#include "MeleeState.h"
#include "FireState.h"
#include "EquipState.h"

// Sets default values for this component's properties
UCharacterStateManagerComponent::UCharacterStateManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	Idle = new IdleState();
	Aim = new AimState();
	Melee = new MeleeState();
	Fire = new FireState();
	Equip = new EquipState();

}

void UCharacterStateManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	SetIsReplicated(true);
	SetState(Idle);
}

void UCharacterStateManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCharacterStateManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCharacterStateManagerComponent, CurrentStateName)
}

bool UCharacterStateManagerComponent::AllowsNextState(EState NextState) const
{
	return CurrentState->AllowsNextState(NextState);
}

void UCharacterStateManagerComponent::ReplicateState(EState State)
{
	// Only replicate state change to nonlocal players since local players will have already changed their own state.
	if (!Cast<ACharacter>(GetOwner())->IsLocallyControlled())
	{
		if (ChangeState(GetStateByName(State), true)) {
			//UE_LOG(LogTemp, Warning, TEXT("REPLICATING TO %s"), *UEnum::GetValueAsString(State));
		}
		
	}
}

// Returns True/False if current state -> target state allowed/unallowed.
bool UCharacterStateManagerComponent::ChangeState(EState State, bool bForceState)
{
	// Only allow local players to change state. Otherwise nonlocal players will prevent OnRep_CurrentState from triggering.
	if (!Cast<ACharacter>(GetOwner())->IsLocallyControlled())
	{
		return false;
	}
	
	return ChangeState(GetStateByName(State), bForceState);
}

// Returns True/False if current state -> target state allowed/unallowed.
bool UCharacterStateManagerComponent::ChangeState(const BaseState* State, bool bForceState)
{
	// To prevent repnotify from changing the state of a player that is still being initialized
	if (CurrentState == nullptr)
	{
		return false;
	}

	if (bForceState || CurrentState->AllowsNextState(State->GetStateName())) {

		// Only locally-controlled players from clients should replicate to server.
		if (GetOwnerRole() != ROLE_Authority && Cast<ACharacter>(GetOwner())->IsLocallyControlled())
		{
			Server_SetState(State->GetStateName());
		}
		
		SetState(State);
		return true;
	}
	
	return false;
}

const BaseState* UCharacterStateManagerComponent::GetStateByName(EState State) const
{
	switch (State) {
	case EState::IDLE:
		return Idle;
		break;
	case EState::AIM:
		return Aim;
		break;
	case EState::MELEE:
		return Melee;
		break;
	case EState::FIRE:
		return Fire;
		break;
	case EState::EQUIP:
		return Equip;
		break;
	default:
		return Idle;
	}
}

void UCharacterStateManagerComponent::SetState(const BaseState* State)
{
	CurrentState = State;
	CurrentStateName = CurrentState->GetStateName();	
}

void UCharacterStateManagerComponent::Server_SetState_Implementation(EState State)
{
	// Must use EState param since BaseState type is not supported for RPCs.
	SetState(GetStateByName(State));
	
}

void UCharacterStateManagerComponent::OnRep_CurrentState()
{
	ReplicateState(CurrentStateName);
}

bool UCharacterStateManagerComponent::IsInEitherState(EState State1, EState State2) const
{
	EState CurrState = GetCurrentStateName();
	if (State1 == CurrState || State2 == CurrState) {
		return true;
	}

	return false;
}

bool UCharacterStateManagerComponent::IsInAnyState(const TArray<EState> &States) const
{
	for (int i = 0; i < States.Num(); i++) {
		if (States[i] == GetCurrentStateName()) {
			return true;
		}
	}

	return false;
}

EState UCharacterStateManagerComponent::GetCurrentStateName() const
{
	if (CurrentState) 
	{
		return CurrentState->GetStateName();
	}
	else 
	{
		return EState::IDLE;
	}
	
}


