// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Game/GsLevelStateGameState.h"

#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Game/GsRankRunSubsystem.h"
#include "Player/Scene/GsRespawnPoint.h"
#include "Settings/GsProjectResourceSettings.h"
#include "UI/Rank/UI_Rank.h"
#include "UEGameJam.h"

AGsLevelStateGameState::AGsLevelStateGameState()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
}

void AGsLevelStateGameState::BeginPlay()
{
	Super::BeginPlay();

	CacheRespawnPoints();
	ActivateInitialCheckpoint();
}

void AGsLevelStateGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UGsRankRunSubsystem* RankRunSubsystem = UGsRankRunSubsystem::Get(this);
	if (!RankRunSubsystem || !RankRunSubsystem->HasActiveRun() || RankRunSubsystem->HasSettledRun())
	{
		return;
	}

	if (RankRunSubsystem->GetRemainingTimeSeconds() > 0.0f)
	{
		return;
	}

	UE_LOG(LogUEGameJam, Log, TEXT("[Rank] Time out triggered."));
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("[Rank] TimeOut"));
	}

	RankRunSubsystem->SettleRun(this, EGsRankSettleReason::TimeOut);

	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		PlayerController->SetIgnoreMoveInput(true);
		PlayerController->SetIgnoreLookInput(true);
		PlayerController->bShowMouseCursor = true;

		FInputModeUIOnly InputMode;
		PlayerController->SetInputMode(InputMode);

		ShowTimeoutRankWidget(PlayerController);
	}

	UGameplayStatics::SetGamePaused(this, true);
}

bool AGsLevelStateGameState::ActivateCheckpointByIndex(int32 CheckpointIndex)
{
	if (bHasRespawnTransform && CheckpointIndex <= CurrentCheckpointIndex)
	{
		return false;
	}

	TObjectPtr<AGsRespawnPoint>* RespawnPointPtr = RespawnPointsByIndex.Find(CheckpointIndex);
	AGsRespawnPoint* RespawnPoint = RespawnPointPtr ? RespawnPointPtr->Get() : nullptr;
	if (!IsValid(RespawnPoint))
	{
		UE_LOG(LogUEGameJam, Warning, TEXT("找不到编号为 %d 的复活点。"), CheckpointIndex);
		return false;
	}

	CurrentCheckpointIndex = CheckpointIndex;
	CurrentRespawnTransform = RespawnPoint->GetActorTransform();
	CurrentRespawnPoint = RespawnPoint;
	bHasRespawnTransform = true;
	if (UGsRankRunSubsystem* RankRunSubsystem = UGsRankRunSubsystem::Get(this))
	{
		RankRunSubsystem->CommitCurrentSegmentKills(CurrentCheckpointIndex);
	}
	return true;
}

bool AGsLevelStateGameState::GetCurrentRespawnTransform(FTransform& OutRespawnTransform) const
{
	if (!bHasRespawnTransform)
	{
		return false;
	}

	OutRespawnTransform = CurrentRespawnTransform;
	return true;
}

void AGsLevelStateGameState::EnsureFallbackRespawnTransform(const FTransform& FallbackTransform)
{
	if (bHasRespawnTransform)
	{
		return;
	}

	CurrentCheckpointIndex = INDEX_NONE;
	CurrentRespawnTransform = FallbackTransform;
	CurrentRespawnPoint = nullptr;
	bHasRespawnTransform = true;
}

bool AGsLevelStateGameState::RegisterDeathAtCurrentRespawnPoint(FText& OutHintText)
{
	OutHintText = FText::GetEmpty();

	if (UGsRankRunSubsystem* RankRunSubsystem = UGsRankRunSubsystem::Get(this))
	{
		RankRunSubsystem->ResetCurrentSegmentKills(CurrentCheckpointIndex);
	}

	if (!IsValid(CurrentRespawnPoint))
	{
		return false;
	}

	const int32 DeathCount = CurrentRespawnPoint->RegisterPlayerDeath();
	return CurrentRespawnPoint->GetHintForDeathCount(DeathCount, OutHintText);
}

void AGsLevelStateGameState::CacheRespawnPoints()
{
	RespawnPointsByIndex.Reset();

	TArray<AActor*> RespawnPointActors;
	UGameplayStatics::GetAllActorsOfClass(this, AGsRespawnPoint::StaticClass(), RespawnPointActors);

	for (AActor* RespawnPointActor : RespawnPointActors)
	{
		AGsRespawnPoint* RespawnPoint = Cast<AGsRespawnPoint>(RespawnPointActor);
		if (!RespawnPoint)
		{
			continue;
		}

		const int32 CheckpointIndex = RespawnPoint->GetCheckpointIndex();
		if (RespawnPointsByIndex.Contains(CheckpointIndex))
		{
			UE_LOG(
				LogUEGameJam,
				Warning,
				TEXT("复活点编号 %d 重复，保留第一个复活点 '%s'，忽略 '%s'。"),
				CheckpointIndex,
				*GetNameSafe(RespawnPointsByIndex[CheckpointIndex].Get()),
				*GetNameSafe(RespawnPoint));
			continue;
		}

		RespawnPointsByIndex.Add(CheckpointIndex, RespawnPoint);
	}
}

void AGsLevelStateGameState::ActivateInitialCheckpoint()
{
	int32 FirstCheckpointIndex = INDEX_NONE;
	bool bHasCheckpoint = false;

	for (const auto& RespawnPointPair : RespawnPointsByIndex)
	{
		if (!IsValid(RespawnPointPair.Value.Get()))
		{
			continue;
		}

		if (!bHasCheckpoint || RespawnPointPair.Key < FirstCheckpointIndex)
		{
			FirstCheckpointIndex = RespawnPointPair.Key;
			bHasCheckpoint = true;
		}
	}

	if (bHasCheckpoint)
	{
		ActivateCheckpointByIndex(FirstCheckpointIndex);
	}
}

void AGsLevelStateGameState::ShowTimeoutRankWidget(APlayerController* PlayerController)
{
	const UGsProjectResourceSettings* ResourceSettings = GetDefault<UGsProjectResourceSettings>();
	const TSubclassOf<UUI_Rank> RankWidgetClass = ResourceSettings ? ResourceSettings->RankWidgetClass : nullptr;
	if (!RankWidgetClass || !PlayerController)
	{
		return;
	}

	if (!RankWidget)
	{
		RankWidget = CreateWidget<UUI_Rank>(PlayerController, RankWidgetClass);
	}

	if (!RankWidget)
	{
		return;
	}

	if (!RankWidget->IsInViewport())
	{
		RankWidget->AddToViewport(20);
	}

	RankWidget->OpenSettlementRank();
}
