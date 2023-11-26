﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "MassSetSmartObjectMoveTargetTask.h"

#include "MassMovementFragments.h"
#include "MassSignalSubsystem.h"
#include "MassSmartObjectFragments.h"
#include "MassStateTreeExecutionContext.h"

bool FMassSetSmartObjectMoveTargetTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(MoveTargetHandle);
	Linker.LinkExternalData(TransformHandle);
	Linker.LinkExternalData(SOUserHandle);
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	Linker.LinkExternalData(MoveParametersHandle);
	
	return true;
}

EStateTreeRunStatus FMassSetSmartObjectMoveTargetTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	// Update MoveTarget location
	//const FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	FMassMoveTargetFragment& MoveTarget = Context.GetExternalData(MoveTargetHandle);
	const FMassSmartObjectUserFragment& SOUserFragment = Context.GetExternalData(SOUserHandle);
	const FMassMovementParameters& MoveParameters = Context.GetExternalData(MoveParametersHandle);

	if (!SOUserFragment.ClaimHandle.IsValid())
		return EStateTreeRunStatus::Failed;

	
	MoveTarget.Center = SOUserFragment.TargetLocation;
	MoveTarget.Forward = SOUserFragment.TargetDirection;
	MoveTarget.SlackRadius = 100.f;
	MoveTarget.DesiredSpeed.Set(MoveParameters.DefaultDesiredSpeed);
	MoveTarget.CreateNewAction(EMassMovementAction::Move, *Context.GetWorld());
	MoveTarget.IntentAtGoal = EMassMovementAction::Stand;

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMassSetSmartObjectMoveTargetTask::Tick(FStateTreeExecutionContext& Context,
                                                            const float DeltaTime) const
{
	// When entity reaches target, mark as complete
	FMassMoveTargetFragment& MoveTarget = Context.GetExternalData(MoveTargetHandle);

	if (MoveTarget.DistanceToGoal <= MoveTarget.SlackRadius)
	{
		MoveTarget.CreateNewAction(EMassMovementAction::Stand, *Context.GetWorld());
		return EStateTreeRunStatus::Succeeded;
	}
	
	return EStateTreeRunStatus::Running;
}
