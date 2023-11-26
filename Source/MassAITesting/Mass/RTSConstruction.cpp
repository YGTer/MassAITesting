﻿
#include "RTSConstruction.h"

#include "MassSmartObjectFragments.h"
#include "RTSAgentTrait.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "SmartObjectComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MassAITesting/BuildingBase.h"
#include "MassAITesting/BuildingManager.h"
#include "MassAITesting/MassAITestingGameMode.h"

//----------------------------------------------------------------------//
// URTSConstructBuilding
//----------------------------------------------------------------------//
URTSConstructBuilding::URTSConstructBuilding()
{
	ObservedType = FRTSConstructFloor::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

void URTSConstructBuilding::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		TArrayView<FRTSAgentFragment> RTSAgents = Context.GetMutableFragmentView<FRTSAgentFragment>();
		TConstArrayView<FMassSmartObjectUserFragment> SOUsers = Context.GetFragmentView<FMassSmartObjectUserFragment>();
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FRTSAgentFragment& RTSAgent = RTSAgents[EntityIndex];
			const FMassSmartObjectUserFragment& SOUser = SOUsers[EntityIndex];
			
			if (const USmartObjectComponent* SmartObjectComponent = SmartObjectSubsystem->GetSmartObjectComponent(SOUser.InteractionHandle))
			{
				ABuildingBase* Actor = SmartObjectComponent->GetOwner<ABuildingBase>();
				ABuildingManager* BuildingManager = Cast<ABuildingManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ABuildingManager::StaticClass()));
				UInstancedStaticMeshComponent* InstancedStaticMeshComp = nullptr;
				if (Actor && BuildingManager)
				{
					// Essentially pick the correct ISM based on the floor level (Floor, Mid, Roof)
					InstancedStaticMeshComp = Actor->CurrentFloor == 0 ? BuildingManager->FloorISM : Actor->CurrentFloor == Actor->Floors-1 ? BuildingManager->RoofISM : BuildingManager->MidISM;
				
					FTransform Transform;
					Transform.SetLocation(FVector(0,0,BuildingManager->FloorHeight*Actor->CurrentFloor)+Actor->GetActorLocation());
					Transform.SetRotation(FRotator(0.0,FMath::RandRange(0,3)*90.0,0.0).Quaternion());
					InstancedStaticMeshComp->AddInstance(Transform, true);
					Actor->CurrentFloor++;
				}

				RTSAgent.BuildingHandle = FSmartObjectHandle::Invalid;
				Context.Defer().RemoveTag<FRTSConstructFloor>(Context.GetEntity(EntityIndex));

				int* ResourceAmount = RTSAgent.Inventory.Find(EResourceType::Rock);
				if (ResourceAmount)
					*ResourceAmount -= 1;
				ResourceAmount = RTSAgent.Inventory.Find(EResourceType::Tree);
				if (ResourceAmount)
					*ResourceAmount -= 1;
				
			}
		}
	});
}

void URTSConstructBuilding::ConfigureQueries()
{
	EntityQuery.AddRequirement<FRTSAgentFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassSmartObjectUserFragment>(EMassFragmentAccess::ReadOnly);
}

void URTSConstructBuilding::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);

	SmartObjectSubsystem = UWorld::GetSubsystem<USmartObjectSubsystem>(Owner.GetWorld());
}