// Copyright

#include "Player/Game/GsRankRunSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Enemy/EnemyCharacter.h"
#include "Enemy/EnemyRespawnSubsystem.h"
#include "HAL/PlatformTime.h"
#include "Player/Game/GsPlayerSaveGame.h"
#include "Settings/GsProjectResourceSettings.h"
#include "UEGameJam.h"

static const TCHAR* GetRankSettleReasonLogText(EGsRankSettleReason Reason)
{
	switch (Reason)
	{
	case EGsRankSettleReason::Completed:
		return TEXT("Completed");
	case EGsRankSettleReason::TimeOut:
		return TEXT("TimeOut");
	case EGsRankSettleReason::Interrupted:
		return TEXT("Interrupted");
	default:
		return TEXT("None");
	}
}

static float LoadRankTimeLimitSeconds()
{
	if (const UGsPlayerSaveGame* SaveGame = UGsPlayerSaveGame::LoadOrCreate())
	{
		const float SavedSeconds = SaveGame->GetRankTimeLimitSeconds();
		if (FMath::IsFinite(SavedSeconds) && SavedSeconds > 0.f)
		{
			return SavedSeconds;
		}
	}

	const UGsProjectResourceSettings* ResourceSettings = GetDefault<UGsProjectResourceSettings>();
	const float ConfiguredSeconds = ResourceSettings ? ResourceSettings->RankTimeLimitSeconds : 180.0f;
	return FMath::IsFinite(ConfiguredSeconds) && ConfiguredSeconds > 0.0f ? ConfiguredSeconds : 180.0f;
}

UGsRankRunSubsystem* UGsRankRunSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UGsRankRunSubsystem>() : nullptr;
}

bool UGsRankRunSubsystem::StartRun(const FString& PlayerName)
{
	const FString TrimmedName = PlayerName.TrimStartAndEnd();
	if (TrimmedName.IsEmpty())
	{
		UE_LOG(LogUEGameJam, Warning, TEXT("[Rank] StartRun failed: empty player name."));
		return false;
	}

	CurrentPlayerName = TrimmedName;
	RunStartRealSeconds = FPlatformTime::Seconds();
	CommittedKillCount = 0;
	CurrentSegmentKillCount = 0;
	CurrentRankTimeLimitSeconds = LoadRankTimeLimitSeconds();
	bHasActiveRun = true;
	bHasSettledRun = false;

	return true;
}

void UGsRankRunSubsystem::RegisterPlayerKill(AEnemyCharacter* KilledEnemy)
{
	if (!bHasActiveRun || bHasSettledRun || !IsValid(KilledEnemy))
	{
		return;
	}

	bool bWillRespawnOnPlayerRespawn = true;
	if (const UEnemyRespawnSubsystem* EnemyRespawnSubsystem = UEnemyRespawnSubsystem::Get(KilledEnemy))
	{
		bWillRespawnOnPlayerRespawn = EnemyRespawnSubsystem->WillEnemyRespawnOnPlayerRespawn(KilledEnemy);
	}

	if (bWillRespawnOnPlayerRespawn)
	{
		++CurrentSegmentKillCount;
	}
	else
	{
		++CommittedKillCount;
	}

	UE_LOG(
		LogUEGameJam,
		Log,
		TEXT("[Rank] Player kill registered. Enemy=%s Persistent=%d SegmentKills=%d CommittedTotal=%d TotalPreview=%d"),
		*GetNameSafe(KilledEnemy),
		bWillRespawnOnPlayerRespawn ? 0 : 1,
		CurrentSegmentKillCount,
		CommittedKillCount,
		CommittedKillCount + CurrentSegmentKillCount);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			3.0f,
			FColor::Green,
			FString::Printf(
				TEXT("[Rank] Kill +1 %s Segment=%d Total=%d"),
				bWillRespawnOnPlayerRespawn ? TEXT("Segment") : TEXT("Committed"),
				CurrentSegmentKillCount,
				CommittedKillCount + CurrentSegmentKillCount));
	}
}

void UGsRankRunSubsystem::CommitCurrentSegmentKills(int32 CheckpointIndex)
{
	if (!bHasActiveRun || bHasSettledRun || CurrentSegmentKillCount <= 0)
	{
		return;
	}

	CommittedKillCount += CurrentSegmentKillCount;

	UE_LOG(
		LogUEGameJam,
		Log,
		TEXT("[Rank] Checkpoint committed segment kills. Added=%d CommittedTotal=%d Checkpoint=%d"),
		CurrentSegmentKillCount,
		CommittedKillCount,
		CheckpointIndex);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			4.0f,
			FColor::Cyan,
			FString::Printf(TEXT("[Rank] Commit Segment=%d Total=%d"), CurrentSegmentKillCount, CommittedKillCount));
	}

	CurrentSegmentKillCount = 0;
}

void UGsRankRunSubsystem::ResetCurrentSegmentKills(int32 CheckpointIndex)
{
	if (!bHasActiveRun || bHasSettledRun || CurrentSegmentKillCount <= 0)
	{
		return;
	}

	UE_LOG(
		LogUEGameJam,
		Log,
		TEXT("[Rank] Death reset segment kills. Cleared=%d CommittedTotal=%d Checkpoint=%d"),
		CurrentSegmentKillCount,
		CommittedKillCount,
		CheckpointIndex);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			4.0f,
			FColor::Orange,
			FString::Printf(TEXT("[Rank] Death Clear Segment=%d Total=%d"), CurrentSegmentKillCount, CommittedKillCount));
	}

	CurrentSegmentKillCount = 0;
}

bool UGsRankRunSubsystem::SettleRun(const UObject* WorldContextObject, EGsRankSettleReason Reason)
{
	if (!bHasActiveRun || CurrentPlayerName.TrimStartAndEnd().IsEmpty())
	{
		UE_LOG(LogUEGameJam, Warning, TEXT("[Rank] Settle failed: no active run/player."));
		return false;
	}

	if (bHasSettledRun)
	{
		UE_LOG(LogUEGameJam, Log, TEXT("[Rank] Settle skipped: already settled."));
		return false;
	}

	bHasSettledRun = true;

	const int32 FinalKillCount = CommittedKillCount + CurrentSegmentKillCount;
	const int32 ElapsedMilliseconds = FMath::Max(0, FMath::RoundToInt(GetElapsedRunTimeSeconds() * 1000.0f));

	int32 Rank = INDEX_NONE;
	UGsRankSaveGame* RankSaveGame = UGsRankSaveGame::LoadOrCreate();
	const bool bSubmitted = RankSaveGame && RankSaveGame->SubmitRankRecord(
		CurrentPlayerName,
		FinalKillCount,
		ElapsedMilliseconds,
		Reason,
		Rank);
	const bool bSaved = bSubmitted && UGsRankSaveGame::Save(RankSaveGame);

	UE_LOG(
		LogUEGameJam,
		Log,
		TEXT("[Rank] Settle Reason=%s Saved=%d Player=%s Kills=%d ElapsedMs=%d Rank=%d"),
		GetRankSettleReasonLogText(Reason),
		bSaved ? 1 : 0,
		*CurrentPlayerName,
		FinalKillCount,
		ElapsedMilliseconds,
		Rank);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			6.0f,
			bSaved ? FColor::Cyan : FColor::Red,
			FString::Printf(
				TEXT("[Rank] Settle %s Save=%d Kills=%d Time=%.2fs Rank=%d"),
				GetRankSettleReasonLogText(Reason),
				bSaved ? 1 : 0,
				FinalKillCount,
				GetElapsedRunTimeSeconds(),
				Rank));
	}

	static_cast<void>(WorldContextObject);
	return bSaved;
}

float UGsRankRunSubsystem::GetElapsedRunTimeSeconds() const
{
	if (!bHasActiveRun || RunStartRealSeconds <= 0.0)
	{
		return 0.0f;
	}

	return FMath::Max(0.0f, static_cast<float>(FPlatformTime::Seconds() - RunStartRealSeconds));
}

float UGsRankRunSubsystem::GetRemainingTimeSeconds() const
{
	return FMath::Max(0.0f, GetRankTimeLimitSeconds() - GetElapsedRunTimeSeconds());
}

float UGsRankRunSubsystem::GetRankTimeLimitSeconds() const
{
	if (FMath::IsFinite(CurrentRankTimeLimitSeconds) && CurrentRankTimeLimitSeconds > 0.f)
	{
		return CurrentRankTimeLimitSeconds;
	}

	return LoadRankTimeLimitSeconds();
}
