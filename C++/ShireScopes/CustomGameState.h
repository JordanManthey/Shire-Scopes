// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CustomGameState.generated.h"

/**
 * This is the base class for all ShireScopes GameStates non native to UE source. 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerArrayUpdate);

UCLASS(Abstract)
class SHIRESCOPES_API ACustomGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

	/** Triggered when the PlayerArray is added to or removed from. Used to notify the Server's GameMode. */
	FOnPlayerArrayUpdate OnPlayerArrayUpdate;

	/** Flags all clients' widgets to prepare for a seamless travel. This is useful for filtering specific widgets after a seamless travel. */
	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_FlagSeamlessTravelWidgets() const;

	/** Caches the current GameState Class in all clients' GameInstance. This is useful for determining where a player traveled from. */
	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_CacheGameStateClass() const;

	/**
	 * @param ID the player's ID.
	 * @returns if this ID is associated with a bot ID.
	 */
	UFUNCTION(BlueprintCallable)
	static bool IsBotID(int32 ID);

	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	/** Triggered when a player logs out from the Server's GameMode. */
	virtual void OnPlayerLogout(APlayerController* Player);

protected:

private:

};
