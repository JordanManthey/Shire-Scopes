// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ShireAIController.generated.h"

class UAIPerceptionComponent;

/**
 * This is the base class for all ShireScopes AIControllers non native to UE source.
 * Refer to BP_ShireAIController for more functionality.
 */
UCLASS()
class SHIRESCOPES_API AShireAIController : public AAIController
{
	GENERATED_BODY()

public:

	/** The radius (in degrees) of the AI's vision limits for detecting objects. */
	UPROPERTY(BlueprintReadOnly)
	float SightRadius;

	/** The minimum bullet spread penalty (scalar) that this bot can take on when firing their RangedWeapon. */
	UPROPERTY(EditDefaultsOnly)
	float MinBaseBulletSpreadPenalty{ 0 };

	/** The maximum bullet spread penalty (scalar) that this bot can take on when firing their RangedWeapon. */
	UPROPERTY(EditDefaultsOnly)
	float MaxBaseBulletSpreadPenalty{ 0 };

	/** The penalty scalar the bot should take on when firing at a moving target. */
	UPROPERTY(EditDefaultsOnly)
	float MovementBaseBulletSpreadPenalty{ 0 };

	/** The team that this bot is on (set by LobbyGameState). */
	int Team{ 0 };
	/** The ID of this bot (set by LobbyGameState). */
	int BotID;
	/** The ID of this bot (set by LobbyGameState). */
	FString BotName;

	/** Prepares this bot and starts its behavior. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void StartBehavior();

	/** @returns the bullet spread stength that should be applied to its bullet trajectory. */
	float GetBulletSpreadStrength() const;

protected:

	/** Stops this bot's behavior tree. */
	UFUNCTION(BlueprintCallable)
	void StopBehaviorTree();

	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	/** Syncs SightRadius to that of this bot's PerceptionComponent SightRadius (Set in BP). */
	void CacheSightRadius();
	
private:
	
	/** The perception component associated with this bot. */
	UAIPerceptionComponent* PerceptionComponent;

};
