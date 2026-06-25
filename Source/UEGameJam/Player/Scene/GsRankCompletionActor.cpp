// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Scene/GsRankCompletionActor.h"

#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Game/GsRankRunSubsystem.h"
#include "Settings/GsProjectResourceSettings.h"
#include "UI/Rank/UI_Rank.h"

AGsRankCompletionActor::AGsRankCompletionActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

bool AGsRankCompletionActor::OpenCompletionRank()
{
	if (UGsRankRunSubsystem* RankRunSubsystem = UGsRankRunSubsystem::Get(this))
	{
		if (RankRunSubsystem->HasActiveRun() && !RankRunSubsystem->HasSettledRun())
		{
			RankRunSubsystem->SettleRun(this, EGsRankSettleReason::Completed);
		}
	}

	const UGsProjectResourceSettings* ResourceSettings = GetDefault<UGsProjectResourceSettings>();
	const TSubclassOf<UUI_Rank> RankWidgetClass = ResourceSettings ? ResourceSettings->RankWidgetClass : nullptr;
	if (!RankWidgetClass)
	{
		return false;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController)
	{
		return false;
	}

	if (!RankWidget)
	{
		RankWidget = CreateWidget<UUI_Rank>(PlayerController, RankWidgetClass);
	}

	if (!RankWidget)
	{
		return false;
	}

	if (!RankWidget->IsInViewport())
	{
		RankWidget->AddToViewport(20);
	}

	RankWidget->OpenSettlementRank();

	PlayerController->bShowMouseCursor = true;
	PlayerController->SetIgnoreMoveInput(true);
	PlayerController->SetIgnoreLookInput(true);

	FInputModeUIOnly InputMode;
	PlayerController->SetInputMode(InputMode);

	UGameplayStatics::SetGamePaused(this, true);

	return true;
}
