// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CustomPlayerController.generated.h"

class ULoadingScreenWidget;
class UGameMessageWidget;

struct FCustomPlayerController_SetupData 
{
	bool LocalPossessionAndBeginPlay_Complete{ false };
	bool FilterSeamlessTravelWidgets_Complete{ false };
};

/**
 * This is the base class for all ShireScopes PlayerControllers non native to UE source.
 */

UCLASS()
class SHIRESCOPES_API ACustomPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	ACustomPlayerController();

	/**
	 * Executes a level travel.
	 * @param LevelName the target level for travel.
	 * @param LoadingMessage the message that the loading screen shows.
	 * @param Options the options to be added to the OptionsString for opening the target level.
	 */
	UFUNCTION(Client, Reliable, BlueprintCallable)
	virtual void Client_TravelToLevel(const FString& LevelName, const FString& LoadingMessage = TEXT(""), const FString& Options = TEXT(""));

	/**
	 * Toggles the loading screen.
	 * @param bIsVisible if the loading screen should be visible or hidden.
	 * @param LoadingMessage the message that the loading screen shows.
	 */
	UFUNCTION(Client, Reliable, BlueprintCallable)
	virtual void Client_ToggleLoadingScreen(bool bIsVisible, const FString& LoadingMessage = TEXT("")) const;

	/**
	 * Shows the game message window.
	 * @param Message the message to be shown.
	 */
	UFUNCTION(Client, Reliable, BlueprintCallable)
	virtual void Client_ShowGameMessage(const FString& Message = "") const;

	/**
	 * Shows the game message window with a message derived from the OptionString.
	 * Useful for displaying messages based on data from the previous level.
	 * @param OptionsString from the GameMode.
	 */
	UFUNCTION(Client, Reliable, BlueprintCallable)
	virtual void Client_ShowGameMessageFromOptions(const FString& OptionsString = TEXT("")) const;

	virtual void PostInitProperties() override;

protected:

	/** An array of Widget classes that should persist with the player controller after a seamless travel. */
	UPROPERTY(EditDefaultsOnly)
	TArray<UClass*> WidgetsToPersistOnSeamlessTravel;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> GameMessageWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> LoadingScreenWidgetClass;

	UPROPERTY(BlueprintReadOnly)
	ULoadingScreenWidget* LoadingScreenWidget;

	UGameMessageWidget* GameMessageWidget;

	/** Used to flag various setup processes as completed. Useful for tracking controller setup and enabling tick queries. */
	FCustomPlayerController_SetupData SetupData;

	/** Gets the data from the Server GameMode's OptionString to display the appropiate GameMessage. */
	UFUNCTION(Server, Reliable)
	virtual void Server_HandleGameMessageFromOptions() const;

	/** Triggered on the Server when the local client has possessed a pawn and begun play. */
	UFUNCTION(Server, Reliable)
	void Server_OnLocalControllerPossessAndBeginPlay() const;
	
	/** Applies various user settings to this controller from the Settings Save. */
	UFUNCTION(BlueprintCallable)
	virtual void ApplyUserSettings();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void AcknowledgePossession(APawn* aPawn) override;

	/** Filters widgets that came from a seamless travel based on the array WidgetsToPersistOnSeamlessTravel. */
	virtual void FilterSeamlessTravelWidgets() const;
	virtual void SetupGameWidgets();
	virtual void CheckFor_LocalPossessionAndBeginPlay();
	virtual void CheckFor_FilterSeamlessTravelWidgets();

private:

};
