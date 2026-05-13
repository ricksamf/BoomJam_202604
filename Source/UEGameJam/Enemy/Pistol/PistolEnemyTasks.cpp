// ===================================================
// 文件：PistolEnemyTasks.cpp
// 说明：手枪敌人专属 Task 实现（PistolAim / PistolFire）。
//       声明位于 Enemy/StateTree/EnemyStateTreeTasks.h。
// ===================================================

#include "EnemyStateTreeTasks.h"
#include "PistolEnemy.h"
#include "StateTreeExecutionContext.h"

////////////////////////////////////////////////////////////////////
// PistolAim

FEnemyPistolAimTask::FEnemyPistolAimTask()
{
	bShouldCallTick = true;
}

EStateTreeRunStatus FEnemyPistolAimTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& /*Transition*/) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	Data.ElapsedTime = 0.f;
	Data.bWarningSpawned = false;

	if (!IsValid(Data.PistolEnemy))
	{
		return EStateTreeRunStatus::Failed;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FEnemyPistolAimTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	if (!IsValid(Data.PistolEnemy))
	{
		return EStateTreeRunStatus::Failed;
	}

	Data.ElapsedTime += DeltaTime;

	// 剩余时间 ≤ WarningLeadTime 时一次性 Spawn 枪口预警特效
	if (!Data.bWarningSpawned && Data.ElapsedTime >= (Data.Duration - Data.WarningLeadTime))
	{
		Data.PistolEnemy->SpawnWarningFX();
		Data.bWarningSpawned = true;
	}

	return (Data.ElapsedTime >= Data.Duration) ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Running;
}

#if WITH_EDITOR
FText FEnemyPistolAimTask::GetDescription(const FGuid&, FStateTreeDataView, const IStateTreeBindingLookup&, EStateTreeNodeFormatting) const
{
	return FText::FromString(TEXT("<b>Pistol Aim</b>"));
}
#endif

////////////////////////////////////////////////////////////////////
// PistolFire

EStateTreeRunStatus FEnemyPistolFireTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& /*Transition*/) const
{
	const FInstanceDataType& Data = Context.GetInstanceData(*this);
	if (!IsValid(Data.PistolEnemy))
	{
		return EStateTreeRunStatus::Failed;
	}

	FVector AimLoc = FVector::ZeroVector;
	if (IsValid(Data.Target))
	{
		AimLoc = Data.Target->GetActorLocation();
	}
	else
	{
		AimLoc = Data.PistolEnemy->GetMuzzleLocation() + Data.PistolEnemy->GetActorForwardVector() * 10000.f;
	}

	Data.PistolEnemy->FireProjectile(AimLoc);
	return EStateTreeRunStatus::Succeeded; // 单帧任务
}

#if WITH_EDITOR
FText FEnemyPistolFireTask::GetDescription(const FGuid&, FStateTreeDataView, const IStateTreeBindingLookup&, EStateTreeNodeFormatting) const
{
	return FText::FromString(TEXT("<b>Pistol Fire</b>"));
}
#endif
