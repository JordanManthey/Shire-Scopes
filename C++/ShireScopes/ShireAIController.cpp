// Fill out your copyright notice in the Description page of Project Settings.

#include "ShireAIController.h"
#include "ShireCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "BrainComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

void AShireAIController::BeginPlay()
{
	Super::BeginPlay();
	CacheSightRadius();
}

void AShireAIController::CacheSightRadius()
{
	PerceptionComponent = GetAIPerceptionComponent();

	FAISenseID id = UAISense::GetSenseID(UAISense_Sight::StaticClass());
	if (id.IsValid())
	{
		UAISenseConfig* senseConfig = PerceptionComponent->GetSenseConfig(id);

		UAISenseConfig_Sight* sightConfig = nullptr;
		if (senseConfig)
		{
			sightConfig = Cast<UAISenseConfig_Sight>(senseConfig);
		}

		if (sightConfig)
		{
			SightRadius = sightConfig->SightRadius;
		}
	}
}

float AShireAIController::GetBulletSpreadStrength() const
{
	const UBlackboardComponent* blackboard = GetBlackboardComponent();
	AShireCharacter* targetPlayer = Cast<AShireCharacter>(blackboard->GetValueAsObject(FName(TEXT("TargetPlayer"))));

	float spreadStrengthScaler = 0;
	if (targetPlayer)
	{
		float targetPlayerSpeed = UKismetMathLibrary::VSizeXY(targetPlayer->GetVelocity());
		float maxWalkSpeed = targetPlayer->GetCharacterMovement()->MaxWalkSpeed;
		float targetPlayerSpeedPercent = targetPlayerSpeed / maxWalkSpeed;
		float targetPlayerMovementPenalty = MovementBaseBulletSpreadPenalty * targetPlayerSpeedPercent;
		float basePenalty = UKismetMathLibrary::RandomFloatInRange(MinBaseBulletSpreadPenalty, MaxBaseBulletSpreadPenalty);
		spreadStrengthScaler = basePenalty + targetPlayerMovementPenalty;
	}
	
	return spreadStrengthScaler;
}

void AShireAIController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

}

void AShireAIController::StopBehaviorTree()
{
	//GetBrainComponent()->StopLogic(TEXT("ROUND OVER"));
	UBehaviorTreeComponent* behaviorTreeComponent = Cast<UBehaviorTreeComponent>(GetBrainComponent());
	behaviorTreeComponent->StopTree();
}

void AShireAIController::StartBehavior_Implementation()
{
	// Defined in BP.
}


