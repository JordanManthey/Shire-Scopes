// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomPlayerController.h"
#include "ShirePlayerController.generated.h"

class UShireHUDWidget;

struct FShirePlayerController_SetupData : FCustomPlayerController_SetupData
{
	bool bSetupCharacterWidgets_Complete{ false };
};

/**
 * This is the base class for all player controllers responsible for possessing a ShireCharacter.
 */

UCLASS()
class SHIRESCOPES_API AShirePlayerController : public ACustomPlayerController
{
	GENERATED_BODY()

public:

	AShirePlayerController();

	/** Toggle the sensitivity type for this player based on @param bUseScopeSensitivity. */
	void ToggleLookSensitivity(bool bUseScopeSensitivity);

protected:

	/** The widget used for displaying game stats. */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> GameStatsWidgetClass;
	UUserWidget* GameStatsWidget;

	/** The widget used for displaying the game HUD. */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> GameHUDWidgetClass;
	UUserWidget* GameHUDWidget;

	/** The widget used for displaying the exit menu. */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> ExitMenuWidgetClass;
	UUserWidget* ExitMenuWidget;

	/** The widget used for displaying the shire character HUD. */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> ShireHUDWidgetClass;
	UPROPERTY(BlueprintReadOnly)
	UShireHUDWidget* ShireHUDWidget;

	/** The widget used for displaying the scope view. */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> ScopeWidgetClass;
	UUserWidget* ScopeWidget;

	/** The scaler to multiply LookSensitivity and ScopeSensitivity by. This allows these settings to stay on a 0-1 scale. */
	UPROPERTY(EditDefaultsOnly)
	float SensitivityScalar{ 10 };

	/** The look sensitivity for when the player is not aiming a weapon. */
	float LookSensitivity { 0.5 };

	/** The look sensitivity for when the player is aiming a weapon. */
	float ScopeSensitivity { 0.5 };

	/** The time delay between a player dying and becoming a spectator. */
	float SpectateModeDelay{ 1 };

	/** The struct used to flag various setup processes as completed. Useful for tracking controller setup and enabling tick queries. */
	FShirePlayerController_SetupData SetupData;

	void ApplyUserSettings() override;

	/** Triggered when a round starts in a match. */
	UFUNCTION()
	void OnRoundStart();

	/** Triggered when a round end in a match. */
	UFUNCTION()
	void OnMatchOver(int WinningTeam);

	/** Triggered after a character death that this controller possesses. */
	UFUNCTION()
	void OnCharacterDeath(AShireCharacter* DeathCharacter);

	/** Notifies the server to spawn a spectator pawn and have this controller possess it. */
	UFUNCTION(Server, Reliable)
	void Server_SpectateMode(AShireCharacter* DeathCharacter);

	/** Toggles the exit menu for this player. */
	UFUNCTION(BlueprintCallable)
	void ToggleExitMenu();

	/** Triggered if this is local controller and has begun play. */
	void OnLocalControllerBeginPlay();

	void ShowGameStats();
	void HideGameStats();
	void BindToGameEvents();
	void CheckFor_SetupCharacterWidgets();
	virtual void SetupShireWidgets();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnPossess(APawn* aPawn) override;
	virtual void AcknowledgePossession(APawn* aPawn) override;
	virtual void SetupInputComponent() override;
	virtual void SetupGameWidgets() override;
	
private:
	
};
