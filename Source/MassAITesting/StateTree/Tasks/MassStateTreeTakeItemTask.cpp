// Fill out your copyright notice in the Description page of Project Settings.


#include "MassStateTreeTakeItemTask.h"

#include "MassEntitySubsystem.h"
#include "MassAITesting/RTSBuildingSubsystem.h"
#include "MassAITesting/Mass/RTSItemTrait.h"

bool FMassStateTreeTakeItemTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkInstanceDataProperty(EntityHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassStateTreeTakeItemTaskInstanceData, ItemHandle));
	
	Linker.LinkExternalData(EntitySubsystemHandle);
	Linker.LinkExternalData(BuildingSubsystemHandle);
	return true;
}

EStateTreeRunStatus FMassStateTreeTakeItemTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const FMassEntityHandle& ItemHandle = Context.GetInstanceData(EntityHandle);
	UMassEntitySubsystem& EntitySubsystem = Context.GetExternalData(EntitySubsystemHandle);
	FMassEntityManager& EntityManager = EntitySubsystem.GetMutableEntityManager();
	URTSBuildingSubsystem& BuildingSubsystem = Context.GetExternalData(BuildingSubsystemHandle);
	
	if (EntityManager.IsEntityValid(ItemHandle))
	{
		const FItemFragment* Item = EntityManager.GetFragmentDataPtr<FItemFragment>(ItemHandle);
		BuildingSubsystem.ItemHashGrid.Remove(ItemHandle, Item->CellLoc);
		EntityManager.Defer().DestroyEntity(ItemHandle);
		return EStateTreeRunStatus::Succeeded;
	}
	return EStateTreeRunStatus::Failed;
}
