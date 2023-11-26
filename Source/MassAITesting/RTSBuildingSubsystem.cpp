// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSBuildingSubsystem.h"

#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "Mass/RTSItemTrait.h"

void URTSBuildingSubsystem::AddBuilding(const FSmartObjectHandle& BuildingRequest, int Floors)
{
	QueuedBuildings.Emplace(FBuilding(BuildingRequest, Floors));
}

bool URTSBuildingSubsystem::ClaimFloor(FSmartObjectHandle& OutBuilding)
{
	bool bSuccess = false;
	if (QueuedBuildings.Num() > 0)
	{
		FBuilding& BuildStruct = QueuedBuildings[0];
		OutBuilding = BuildStruct.BuildingRequest;
		BuildStruct.FloorsNeeded--;
		
		if (BuildStruct.FloorsNeeded <= 0)
			QueuedBuildings.RemoveAt(0);
		bSuccess = true;
	}
	return bSuccess;
}

bool URTSBuildingSubsystem::FindItem(const FVector& Location, float Radius, EResourceType ResourceType, FMassEntityHandle& OutItemHandle) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("ResourceFindItem"));
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	
	// Query items in radius
	TArray<FMassEntityHandle> Entities;
	const FBox Bounds(Location - FVector(Radius, Radius, 0.f), Location + FVector(Radius, Radius, 0.f));
	ItemHashGrid.Query(Bounds,Entities);

	// Sort potential entities by distance
	Entities.Sort([&EntityManager, &Location](const FMassEntityHandle& A, const FMassEntityHandle& B)
	{
		const FVector& LocA = EntityManager.GetFragmentDataPtr<FTransformFragment>(A)->GetTransform().GetLocation();
		const FVector& LocB = EntityManager.GetFragmentDataPtr<FTransformFragment>(B)->GetTransform().GetLocation();
		
		return FVector::Dist(Location, LocA) < FVector::Dist(Location, LocB);
	});

	// Perform filtering
	for(const FMassEntityHandle& Entity : Entities)
	{
		if (const FItemFragment* Item = EntityManager.GetFragmentDataPtr<FItemFragment>(Entity))
		{
			if (Item->ItemType == ResourceType && !Item->bClaimed)
			{
				OutItemHandle = Entity;
				return EntityManager.IsEntityValid(Entity);
			}
		}
	}
	
	return false;
}

bool URTSBuildingSubsystem::ClaimResource(FSmartObjectHandle& OutResourceHandle)
{
	bool bSuccess = false;
	if (QueuedResources.Num() > 0)
	{
		OutResourceHandle = QueuedResources[0];
		//UE_LOG(LogTemp, Error, TEXT("Building ID: %s | Floors remaining: %d"), *LexToString(BuildStruct.BuildingRequest), BuildStruct.FloorsNeeded)
		QueuedResources.RemoveAt(0);
		bSuccess = true;
	}
	return bSuccess;
}

void URTSBuildingSubsystem::AddResourceQueue(FSmartObjectHandle& SOHandle)
{
	if (QueuedResources.Find(SOHandle) == INDEX_NONE)
	{
		QueuedResources.Emplace(SOHandle);
	}
}

void URTSBuildingSubsystem::AddRTSAgent(const FMassEntityHandle& Entity)
{
	RTSAgents.Emplace(Entity);
}

void URTSBuildingSubsystem::SelectClosestAgent(const FVector& Location)
{
	float ClosestDistance = -1;
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	for(const FMassEntityHandle& Entity : RTSAgents)
	{
		const FVector& EntityLocation = EntityManager.GetFragmentDataPtr<FTransformFragment>(Entity)->GetTransform().GetLocation();
		float Distance = FVector::Dist(EntityLocation, Location);
		if (ClosestDistance == -1 || Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			RTSAgent = Entity;
		}
	}
}

void URTSBuildingSubsystem::GetAgentLocation(FVector& OutLocation)
{
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	if (EntitySubsystem && RTSAgent.IsValid())
	{
		OutLocation = EntityManager.GetFragmentDataPtr<FTransformFragment>(RTSAgent)->GetTransform().GetLocation();
	}
}

void URTSBuildingSubsystem::GetAgentInformation(FRTSAgentFragment& OutAgentInfo)
{
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	if (EntitySubsystem && RTSAgent.IsValid())
	{
		OutAgentInfo = *EntityManager.GetFragmentDataPtr<FRTSAgentFragment>(RTSAgent);
	}
}
