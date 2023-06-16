// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneCaptureComponent2D.h"
#include "ShireSceneCaptureComponent2D.generated.h"

/**
 * DEPRECATED: Was used to fix a bug with UE's USceneCaptureComponent2D class. No longer using this functionality.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup=(Camera), meta=(BlueprintSpawnableComponent))
class SHIRESCOPES_API UShireSceneCaptureComponent2D : public USceneCaptureComponent2D
{
	GENERATED_BODY()

		virtual const AActor* GetViewOwner() const override { return GetOwner(); }
	
};
