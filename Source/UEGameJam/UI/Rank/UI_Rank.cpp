// Copyright


#include "UI_Rank.h"

#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Game/GsRankRunSubsystem.h"
#include "Player/Game/GsRankSaveGame.h"
#include "UI/Rank/UI_RankPlayerItem.h"

static constexpr int32 MaxRankDisplayCount = 10;
static constexpr int32 TopRankDisplayCount = 3;
static constexpr int32 NearbyRankDisplayCount = 7;
static constexpr int32 NearbyRankHalfCount = 3;

static FString GetNormalizedRankUiPlayerName(const FString& Name)
{
	return Name.TrimStartAndEnd().ToLower();
}

static bool IsLegacyRankUiPlaceholderRecord(const FGsRankPlayerRecord& PlayerRecord)
{
	return PlayerRecord.SettleReason == EGsRankSettleReason::None
		&& PlayerRecord.KillCount == 0
		&& PlayerRecord.ElapsedMilliseconds == 0
		&& PlayerRecord.SubmissionOrder == 0;
}

static bool CompareRankUiPlayerRecord(const FGsRankPlayerRecord& Left, const FGsRankPlayerRecord& Right)
{
	if (Left.KillCount != Right.KillCount)
	{
		return Left.KillCount > Right.KillCount;
	}

	if (Left.ElapsedMilliseconds != Right.ElapsedMilliseconds)
	{
		return Left.ElapsedMilliseconds < Right.ElapsedMilliseconds;
	}

	return Left.SubmissionOrder > Right.SubmissionOrder;
}

void UUI_Rank::OpenRank()
{
	SetVisibility(ESlateVisibility::Visible);
	SetBackMainMenuButtonVisible(false);
	RefreshRank();
}

void UUI_Rank::OpenSettlementRank()
{
	SetVisibility(ESlateVisibility::Visible);
	SetBackMainMenuButtonVisible(true);
	RefreshRank();
}

void UUI_Rank::RefreshRank()
{
	if (!RankListBox)
	{
		return;
	}

	RankListBox->ClearChildren();
	RankItems.Reset();
	AnimatedItems.Reset();
	bIsOpeningAnimationPlaying = false;

	if (!RankPlayerItemClass)
	{
		return;
	}

	UGsRankSaveGame* RankSaveGame = UGsRankSaveGame::LoadOrCreate();
	if (!RankSaveGame)
	{
		return;
	}

	TArray<FGsRankPlayerRecord> Records = RankSaveGame->PlayerRecords;
	Records.RemoveAll([](const FGsRankPlayerRecord& PlayerRecord)
	{
		return IsLegacyRankUiPlaceholderRecord(PlayerRecord);
	});
	Records.Sort(CompareRankUiPlayerRecord);

	const FString CurrentPlayerName = GetActiveCurrentPlayerName();
	const int32 CurrentPlayerIndex = FindCurrentPlayerIndex(Records, CurrentPlayerName);

	TArray<int32> DisplayRecordIndices;
	BuildDisplayRecordIndices(Records, CurrentPlayerIndex, DisplayRecordIndices);

	for (int32 DisplayRecordIndex : DisplayRecordIndices)
	{
		if (!Records.IsValidIndex(DisplayRecordIndex))
		{
			continue;
		}

		UUI_RankPlayerItem* RankItem = nullptr;
		if (APlayerController* OwningPlayer = GetOwningPlayer())
		{
			RankItem = CreateWidget<UUI_RankPlayerItem>(OwningPlayer, RankPlayerItemClass);
		}
		else if (UWorld* World = GetWorld())
		{
			RankItem = CreateWidget<UUI_RankPlayerItem>(World, RankPlayerItemClass);
		}

		if (!RankItem)
		{
			continue;
		}

		const bool bIsCurrentPlayer = DisplayRecordIndex == CurrentPlayerIndex;
		RankItem->SetupRankItem(DisplayRecordIndex + 1, Records[DisplayRecordIndex], bIsCurrentPlayer);
		RankListBox->AddChildToVerticalBox(RankItem);
		RankItems.Add(RankItem);
	}

	PlayOpeningAnimation();
}

void UUI_Rank::SetCurrentPlayerName(const FString& InCurrentPlayerName)
{
	ManualCurrentPlayerName = InCurrentPlayerName.TrimStartAndEnd();

	if (RankListBox)
	{
		RefreshRank();
	}
}

void UUI_Rank::NativeConstruct()
{
	Super::NativeConstruct();

	if (BackMainMenuButton)
	{
		BackMainMenuButton->OnClicked.RemoveDynamic(this, &UUI_Rank::HandleBackMainMenuClicked);
		BackMainMenuButton->OnClicked.AddDynamic(this, &UUI_Rank::HandleBackMainMenuClicked);
	}

	OpenRank();
}

void UUI_Rank::NativeDestruct()
{
	if (BackMainMenuButton)
	{
		BackMainMenuButton->OnClicked.RemoveDynamic(this, &UUI_Rank::HandleBackMainMenuClicked);
	}

	AnimatedItems.Reset();
	bIsOpeningAnimationPlaying = false;

	Super::NativeDestruct();
}

void UUI_Rank::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bIsOpeningAnimationPlaying)
	{
		return;
	}

	AnimationElapsedTime += InDeltaTime;
	bool bAllItemsFinished = true;

	for (const FRankAnimatedItem& AnimatedItem : AnimatedItems)
	{
		UUI_RankPlayerItem* RankItem = AnimatedItem.Item.Get();
		if (!RankItem)
		{
			continue;
		}

		const float ItemElapsedTime = AnimationElapsedTime - AnimatedItem.StartTime;
		if (ItemElapsedTime < 0.0f)
		{
			RankItem->SetRenderOpacity(0.0f);
			RankItem->SetRenderTranslation(AnimatedItem.bIsCurrentPlayer ? FVector2D::ZeroVector : FVector2D(0.0f, RowAppearOffsetY));
			bAllItemsFinished = false;
			continue;
		}

		const float AppearAlpha = FMath::Clamp(ItemElapsedTime / RowAppearDuration, 0.0f, 1.0f);
		const float EasedAlpha = 1.0f - FMath::Pow(1.0f - AppearAlpha, 3.0f);
		const float OffsetY = AnimatedItem.bIsCurrentPlayer ? 0.0f : FMath::Lerp(RowAppearOffsetY, 0.0f, EasedAlpha);

		RankItem->SetRenderTranslation(FVector2D(0.0f, OffsetY));

		float TargetOpacity = EasedAlpha;
		float RequiredDuration = RowAppearDuration;
		if (AnimatedItem.bIsCurrentPlayer && CurrentPlayerFlashDuration > 0.0f && CurrentPlayerFlashCount > 0.0f)
		{
			RequiredDuration += CurrentPlayerFlashDuration;

			const float FlashElapsedTime = ItemElapsedTime - RowAppearDuration;
			if (FlashElapsedTime >= 0.0f && FlashElapsedTime <= CurrentPlayerFlashDuration)
			{
				const float FlashPhase = FlashElapsedTime / CurrentPlayerFlashDuration * CurrentPlayerFlashCount * 2.0f * PI;
				const float FlashAlpha = 0.5f + 0.5f * FMath::Cos(FlashPhase);
				TargetOpacity = FMath::Lerp(CurrentPlayerFlashMinOpacity, 1.0f, FlashAlpha);
			}
		}

		RankItem->SetRenderOpacity(TargetOpacity);

		if (ItemElapsedTime < RequiredDuration)
		{
			bAllItemsFinished = false;
		}
	}

	if (bAllItemsFinished)
	{
		FinishOpeningAnimation();
	}
}

void UUI_Rank::PlayOpeningAnimation()
{
	AnimatedItems.Reset();
	AnimationElapsedTime = 0.0f;

	if (RankItems.Num() <= 0)
	{
		bIsOpeningAnimationPlaying = false;
		return;
	}

	UUI_RankPlayerItem* CurrentPlayerItem = nullptr;
	float NextStartTime = 0.0f;

	for (UUI_RankPlayerItem* RankItem : RankItems)
	{
		if (!RankItem)
		{
			continue;
		}

		RankItem->SetRenderOpacity(0.0f);

		if (RankItem->IsCurrentPlayerItem())
		{
			RankItem->SetRenderTranslation(FVector2D::ZeroVector);
			CurrentPlayerItem = RankItem;
			continue;
		}

		RankItem->SetRenderTranslation(FVector2D(0.0f, RowAppearOffsetY));
		AddAnimatedItem(RankItem, NextStartTime, false);
		NextStartTime += RowAppearInterval;
	}

	if (CurrentPlayerItem)
	{
		AddAnimatedItem(CurrentPlayerItem, NextStartTime, true);
	}

	bIsOpeningAnimationPlaying = AnimatedItems.Num() > 0;
}

void UUI_Rank::AddAnimatedItem(UUI_RankPlayerItem* Item, float StartTime, bool bIsCurrentPlayer)
{
	if (!Item)
	{
		return;
	}

	FRankAnimatedItem AnimatedItem;
	AnimatedItem.Item = Item;
	AnimatedItem.StartTime = StartTime;
	AnimatedItem.bIsCurrentPlayer = bIsCurrentPlayer;
	AnimatedItems.Add(AnimatedItem);
}

void UUI_Rank::FinishOpeningAnimation()
{
	for (UUI_RankPlayerItem* RankItem : RankItems)
	{
		if (!RankItem)
		{
			continue;
		}

		RankItem->SetRenderOpacity(1.0f);
		RankItem->SetRenderTranslation(FVector2D::ZeroVector);
	}

	AnimatedItems.Reset();
	bIsOpeningAnimationPlaying = false;
}

void UUI_Rank::SetBackMainMenuButtonVisible(bool bVisible)
{
	if (BackMainMenuButton)
	{
		BackMainMenuButton->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UUI_Rank::HandleBackMainMenuClicked()
{
	UGameplayStatics::SetGamePaused(this, false);

	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		PlayerController->SetIgnoreMoveInput(false);
		PlayerController->SetIgnoreLookInput(false);
		PlayerController->bShowMouseCursor = false;

		FInputModeGameOnly InputMode;
		PlayerController->SetInputMode(InputMode);
	}

	RemoveFromParent();

	if (!MainMenuLevelName.IsNone())
	{
		UGameplayStatics::OpenLevel(this, MainMenuLevelName);
	}
}

void UUI_Rank::BuildDisplayRecordIndices(const TArray<FGsRankPlayerRecord>& Records, int32 CurrentPlayerIndex, TArray<int32>& OutIndices) const
{
	OutIndices.Reset();

	if (Records.Num() <= MaxRankDisplayCount || CurrentPlayerIndex == INDEX_NONE)
	{
		const int32 DisplayCount = FMath::Min(Records.Num(), MaxRankDisplayCount);
		for (int32 Index = 0; Index < DisplayCount; ++Index)
		{
			OutIndices.Add(Index);
		}

		return;
	}

	TSet<int32> SelectedIndices;
	for (int32 Index = 0; Index < TopRankDisplayCount && Index < Records.Num(); ++Index)
	{
		SelectedIndices.Add(Index);
	}

	const int32 MaxWindowStart = FMath::Max(0, Records.Num() - NearbyRankDisplayCount);
	const int32 WindowStart = FMath::Clamp(CurrentPlayerIndex - NearbyRankHalfCount, 0, MaxWindowStart);
	const int32 WindowEnd = FMath::Min(Records.Num() - 1, WindowStart + NearbyRankDisplayCount - 1);
	for (int32 Index = WindowStart; Index <= WindowEnd; ++Index)
	{
		SelectedIndices.Add(Index);
	}

	for (int32 Index = 0; SelectedIndices.Num() < MaxRankDisplayCount && Index < Records.Num(); ++Index)
	{
		SelectedIndices.Add(Index);
	}

	OutIndices = SelectedIndices.Array();
	OutIndices.Sort();
}

int32 UUI_Rank::FindCurrentPlayerIndex(const TArray<FGsRankPlayerRecord>& Records, const FString& PlayerName) const
{
	const FString NormalizedPlayerName = GetNormalizedRankUiPlayerName(PlayerName);
	if (NormalizedPlayerName.IsEmpty())
	{
		return INDEX_NONE;
	}

	for (int32 Index = 0; Index < Records.Num(); ++Index)
	{
		if (GetNormalizedRankUiPlayerName(Records[Index].PlayerName) == NormalizedPlayerName)
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

FString UUI_Rank::GetActiveCurrentPlayerName() const
{
	if (const UGsRankRunSubsystem* RankRunSubsystem = UGsRankRunSubsystem::Get(this))
	{
		const FString RunPlayerName = RankRunSubsystem->GetCurrentPlayerName().TrimStartAndEnd();
		if (!RunPlayerName.IsEmpty())
		{
			return RunPlayerName;
		}
	}

	return ManualCurrentPlayerName.TrimStartAndEnd();
}
