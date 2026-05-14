// ===================================================
// 文件：MachineGunEnemyTasks.cpp
// 说明：机枪敌人专属 Task 实现（MGWarmup / MGBurst）。
//       声明位于 Enemy/StateTree/EnemyStateTreeTasks.h。
// ===================================================

#include "EnemyStateTreeTasks.h"
#include "MachineGunEnemy.h"
#include "StateTreeExecutionContext.h"

////////////////////////////////////////////////////////////////////
// MGWarmup

FEnemyMGWarmupTask::FEnemyMGWarmupTask()
{
	bShouldCallTick = true;
}

EStateTreeRunStatus FEnemyMGWarmupTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& /*Transition*/) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	Data.ElapsedTime = 0.f;
	Data.bWarningSpawned = false;

	if (!IsValid(Data.MGEnemy))
	{
		return EStateTreeRunStatus::Failed;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FEnemyMGWarmupTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	Data.ElapsedTime += DeltaTime;

	// 剩余时间 ≤ WarningLeadTime 时一次性 Spawn 枪口预警特效
	if (!Data.bWarningSpawned && IsValid(Data.MGEnemy)
		&& Data.ElapsedTime >= (Data.Duration - Data.WarningLeadTime))
	{
		Data.MGEnemy->SpawnWarningFX();
		Data.bWarningSpawned = true;
	}

	return (Data.ElapsedTime >= Data.Duration) ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Running;
}

#if WITH_EDITOR
FText FEnemyMGWarmupTask::GetDescription(const FGuid&, FStateTreeDataView, const IStateTreeBindingLookup&, EStateTreeNodeFormatting) const
{
	return FText::FromString(TEXT("<b>MG Warmup</b>"));
}
#endif

////////////////////////////////////////////////////////////////////
// MGBurst

FEnemyMGBurstTask::FEnemyMGBurstTask()
{
	bShouldCallTick = true;
}

EStateTreeRunStatus FEnemyMGBurstTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& /*Transition*/) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	Data.ElapsedTime = 0.f;
	Data.ShotAccumulator = 0.f;

	if (!IsValid(Data.MGEnemy))
	{
		return EStateTreeRunStatus::Failed;
	}

	Data.MGEnemy->SetTrackingEnabled(true, Data.Target);
	// Burst 期间一次性触发 BurstMontage,Montage 资产里 Section 循环维持节奏。
	// 子弹由 Montage 内 "Fire" AnimNotify 驱动 spawn(见 AMachineGunEnemy::HandleFireNotify),
	// 所以下面 Tick 不再主动 FireOneBullet,只统计 Burst 总时长。
	Data.MGEnemy->PlayAttackMontage();
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FEnemyMGBurstTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	if (!IsValid(Data.MGEnemy))
	{
		return EStateTreeRunStatus::Failed;
	}

	Data.ElapsedTime += DeltaTime;
	return (Data.ElapsedTime >= Data.Duration) ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Running;
}

void FEnemyMGBurstTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& /*Transition*/) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	if (IsValid(Data.MGEnemy))
	{
		Data.MGEnemy->SetTrackingEnabled(false, nullptr);
		Data.MGEnemy->StopAttackMontage();
	}
}

#if WITH_EDITOR
FText FEnemyMGBurstTask::GetDescription(const FGuid&, FStateTreeDataView, const IStateTreeBindingLookup&, EStateTreeNodeFormatting) const
{
	return FText::FromString(TEXT("<b>MG Burst</b>"));
}
#endif
