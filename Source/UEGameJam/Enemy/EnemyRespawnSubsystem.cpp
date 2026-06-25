// ===================================================
// 文件：EnemyRespawnSubsystem.cpp
// ===================================================

#include "EnemyRespawnSubsystem.h"
#include "EnemyCharacter.h"
#include "Player/Character/GsPlayer.h"
#include "Player/Game/GsLevelStateGameState.h"
#include "Player/Game/GsRankRunSubsystem.h"
#include "Player/Scene/GsRespawnPoint.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UEnemyRespawnSubsystem* UEnemyRespawnSubsystem::Get(const UObject* WorldContext)
{
	if (!WorldContext || !GEngine)
	{
		return nullptr;
	}
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
	{
		return World->GetSubsystem<UEnemyRespawnSubsystem>();
	}
	return nullptr;
}

void UEnemyRespawnSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// 绑定玩家 OnRespawn。多人/多 player 此处只取 0 号本地玩家——单机 jam 足够。
	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(&InWorld, 0))
	{
		if (AGsPlayer* Player = Cast<AGsPlayer>(PlayerPawn))
		{
			Player->OnRespawn.AddDynamic(this, &UEnemyRespawnSubsystem::HandlePlayerRespawn);
		}
	}
}

void UEnemyRespawnSubsystem::RegisterEnemy(AEnemyCharacter* Enemy, int32& InOutRecordId)
{
	if (!Enemy)
	{
		return;
	}

	// 重生流程派生的实例：ID 已被 RespawnAllDead 预设。仅复活该 Record。
	if (InOutRecordId >= 0 && Records.IsValidIndex(InOutRecordId))
	{
		Records[InOutRecordId].bIsDead = false;
		return;
	}

	FEnemyRespawnRecord Rec;
	Rec.EnemyClass = Enemy->GetClass();
	Rec.SpawnTransform = Enemy->GetActorTransform();
	Rec.OwningCheckpoint = Enemy->OwningCheckpointOverride;
	if (Rec.OwningCheckpoint < 0)
	{
		Rec.OwningCheckpoint = InferOwningCheckpoint(Rec.SpawnTransform);
	}
	Rec.bIsDead = false;
	Rec.SourceName = Enemy->GetFName();

	InOutRecordId = Records.Add(Rec);
}

void UEnemyRespawnSubsystem::MarkDead(int32 RecordId)
{
	if (Records.IsValidIndex(RecordId))
	{
		Records[RecordId].bIsDead = true;
	}
}

void UEnemyRespawnSubsystem::RespawnAllDead()
{
	if (!bRespawnEnabled)
	{
		UE_LOG(LogTemp, Log, TEXT("[EnemyRespawn] RespawnAllDead skipped: respawn disabled"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 复活规则：怪物 OwningCheckpoint 必须 **严格大于** 玩家当前使用的 CheckpointIndex 才复活。
	// 玩家当前段及之前的死怪不再复活。
	int32 CurrentCheckpoint = INDEX_NONE;
	if (AGsLevelStateGameState* LevelState = World->GetGameState<AGsLevelStateGameState>())
	{
		CurrentCheckpoint = LevelState->GetCurrentCheckpointIndex();
	}

	int32 RespawnedCount = 0;
	int32 SkippedCount   = 0;
	for (int32 i = 0; i < Records.Num(); ++i)
	{
		FEnemyRespawnRecord& Rec = Records[i];
		if (!Rec.bIsDead || !Rec.EnemyClass)
		{
			continue;
		}

		if (!Rec.bRespawnEnabled)
		{
			++SkippedCount;
			continue;
		}

		if (Rec.OwningCheckpoint <= CurrentCheckpoint)
		{
			++SkippedCount;
			continue;
		}

		// SpawnActorDeferred：actor 已创建但 BeginPlay 暂未跑，先填 RespawnRecordId 再 FinishSpawning。
		AEnemyCharacter* New = World->SpawnActorDeferred<AEnemyCharacter>(
			Rec.EnemyClass,
			Rec.SpawnTransform,
			/*Owner*/nullptr,
			/*Instigator*/nullptr,
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
		if (!New)
		{
			UE_LOG(LogTemp, Warning, TEXT("[EnemyRespawn] SpawnActorDeferred 失败：Class=%s"), *GetNameSafe(Rec.EnemyClass));
			continue;
		}

		New->SetRespawnRecordId(i);
		New->OwningCheckpointOverride = Rec.OwningCheckpoint;
		New->FinishSpawning(Rec.SpawnTransform);

		Rec.bIsDead = false;
		++RespawnedCount;
	}

	UE_LOG(LogTemp, Log, TEXT("[EnemyRespawn] RespawnAllDead: respawned=%d skipped=%d (CurrentCP=%d)"),
		RespawnedCount, SkippedCount, CurrentCheckpoint);
}

void UEnemyRespawnSubsystem::SetRespawnEnabled(bool bEnabled)
{
	bRespawnEnabled = bEnabled;

	if (!bRespawnEnabled)
	{
		int32 CurrentCheckpoint = INDEX_NONE;
		if (UWorld* World = GetWorld())
		{
			if (AGsLevelStateGameState* LevelState = World->GetGameState<AGsLevelStateGameState>())
			{
				CurrentCheckpoint = LevelState->GetCurrentCheckpointIndex();
			}
		}

		if (UGsRankRunSubsystem* RankRunSubsystem = UGsRankRunSubsystem::Get(this))
		{
			RankRunSubsystem->CommitCurrentSegmentKills(CurrentCheckpoint);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[EnemyRespawn] Respawn enabled set to %d"), bRespawnEnabled ? 1 : 0);
}

void UEnemyRespawnSubsystem::DisableRespawnForCheckpoint(int32 CheckpointIndex)
{
	int32 DisabledCount = 0;
	for (FEnemyRespawnRecord& Rec : Records)
	{
		if (Rec.OwningCheckpoint == CheckpointIndex && Rec.bRespawnEnabled)
		{
			Rec.bRespawnEnabled = false;
			Rec.bIsDead = false;
			++DisabledCount;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[EnemyRespawn] Disabled respawn for checkpoint %d: records=%d"),
		CheckpointIndex, DisabledCount);
}

void UEnemyRespawnSubsystem::SetEnemyRespawnEnabled(const UObject* WorldContext, bool bEnabled)
{
	if (UEnemyRespawnSubsystem* RespawnSubsystem = Get(WorldContext))
	{
		RespawnSubsystem->SetRespawnEnabled(bEnabled);
	}
}

void UEnemyRespawnSubsystem::DisableEnemyRespawnForCheckpoint(const UObject* WorldContext, int32 CheckpointIndex)
{
	if (UEnemyRespawnSubsystem* RespawnSubsystem = Get(WorldContext))
	{
		RespawnSubsystem->DisableRespawnForCheckpoint(CheckpointIndex);
	}
}

void UEnemyRespawnSubsystem::HandlePlayerRespawn()
{
	RespawnAllDead();
}

int32 UEnemyRespawnSubsystem::InferOwningCheckpoint(const FTransform& Where) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return -1;
	}

	TArray<AActor*> RpActors;
	UGameplayStatics::GetAllActorsOfClass(World, AGsRespawnPoint::StaticClass(), RpActors);

	float BestDistSq = TNumericLimits<float>::Max();
	int32 BestIdx = -1;
	for (AActor* A : RpActors)
	{
		const AGsRespawnPoint* Rp = Cast<AGsRespawnPoint>(A);
		if (!Rp)
		{
			continue;
		}
		const float D = FVector::DistSquared(Rp->GetActorLocation(), Where.GetLocation());
		if (D < BestDistSq)
		{
			BestDistSq = D;
			BestIdx = Rp->GetCheckpointIndex();
		}
	}
	return BestIdx;
}

void UEnemyRespawnSubsystem::EnemyRespawnDump() const
{
	UE_LOG(LogTemp, Log, TEXT("[EnemyRespawn] === Records: %d ==="), Records.Num());
	for (int32 i = 0; i < Records.Num(); ++i)
	{
		const FEnemyRespawnRecord& R = Records[i];
		UE_LOG(LogTemp, Log, TEXT("  [%d] Class=%s  CP=%d  Dead=%d  Respawn=%d  Src=%s"),
			i, *GetNameSafe(R.EnemyClass), R.OwningCheckpoint, R.bIsDead ? 1 : 0, R.bRespawnEnabled ? 1 : 0, *R.SourceName.ToString());
	}
}
