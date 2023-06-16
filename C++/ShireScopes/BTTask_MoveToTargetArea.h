// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlueprintBase.h"
#include "BTTask_MoveToTargetArea.generated.h"

/**
 * This is the base class the AI's movement tasks which provides the ability return a curved path to a target location for more natural pathfinding.
 */
UCLASS()
class SHIRESCOPES_API UBTTask_MoveToTargetArea : public UBTTask_BlueprintBase
{
	GENERATED_BODY()

public:

protected:

	/**
	 * A helper function that calculates a location along a perpendicular line from the midpoint of 2 given locations. 
	 * Used in GetBezierSubpathPoints(...) to generate bezier curve anchors.
	 * @param LocationA the starting location.
	 * @param LocationB the ending location.
	 * @param Distance the offset from the midpoint along the perpendicular line.
	 * @returns the perpendicular location.
	 */
	UFUNCTION(BlueprintCallable)
	FVector GetPerpendicularMidpointFromDistance(const FVector& LocationA, const FVector& LocationB, float Distance) const;

	/**
	 * Generates the consecutive locations from a bezier curve between 2 given locations.
	 * @param LocationA the starting location.
	 * @param LocationB the ending location.
	 * @param SubpointDistance the distance between each point along the bezier path.
	 * @returns an array of locations that follow the path of the bezier curve.
	 */
	UFUNCTION(BlueprintCallable)
	TArray<FVector> GetBezierSubpathPoints(const FVector& LocationA, const FVector& LocationB, float SubpointDistance = 150) const;

private:
	
};
