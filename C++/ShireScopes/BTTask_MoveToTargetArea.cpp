// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_MoveToTargetArea.h"

FVector UBTTask_MoveToTargetArea::GetPerpendicularMidpointFromDistance(const FVector& LocationA, const FVector& LocationB, float DistanceFromMidpoint) const
{
	FVector midpoint = (LocationA + LocationB) / 2;
	float slope = (LocationB.Y - LocationA.Y) / (LocationB.X - LocationA.X);
	float perpendicularSlope = -1 / slope;

	// Find the 2 intersection points that the perpendicular line makes with a circle (where radius = distance) centered at the midpoint.

	// x values for intersecting solution
	float x1 = midpoint.X + sqrt((pow(DistanceFromMidpoint, 2) / (1 + (1 / pow(slope, 2)))));
	float x2 = midpoint.X - sqrt((pow(DistanceFromMidpoint, 2) / (1 + (1 / pow(slope, 2)))));

	// Use x values to get corresponding y values from perpendicular line.

	// Perpendicular line
	// y - midpoint.y = perpendicularSlope * (x - midpoint.x)
	// y = perpendicularSlope (x - midpoint.x) + midpoint.y;
	float y1 = (perpendicularSlope * (x1 - midpoint.X)) + midpoint.Y;
	float y2 = (perpendicularSlope * (x2 - midpoint.X)) + midpoint.Y;

	if (FMath::RandRange(0, 1) < 0.5)
	{
		return FVector(x1, y1, midpoint.Z);
	}
	else
	{
		return FVector(x2, y2, midpoint.Z);
	}

}

TArray<FVector> UBTTask_MoveToTargetArea::GetBezierSubpathPoints(const FVector& StartLocation, const FVector& EndLocation, float SubpointDistance) const
{
	float pathDistance = FVector::Distance(StartLocation, EndLocation);
	float distanceFromMidpoint = FMath::RandRange(0.2 * pathDistance, 0.5 * pathDistance);

	FVector controlPoint2 = GetPerpendicularMidpointFromDistance(StartLocation, EndLocation, distanceFromMidpoint);
	FVector controlPoint3 = GetPerpendicularMidpointFromDistance(StartLocation, EndLocation, distanceFromMidpoint);

	TArray<FVector> samplePointsOut;
	FVector controlPoints[4];

	controlPoints[0] = StartLocation;
	controlPoints[1] = controlPoint2;
	controlPoints[2] = controlPoint3;
	controlPoints[3] = EndLocation;

	int numSamplePoints = FMath::Max(2, floor(pathDistance / SubpointDistance));

	double pathLength = FVector::EvaluateBezier(controlPoints, numSamplePoints, samplePointsOut);
	return samplePointsOut;
}
