// Copyright

#include "UI_RankPlayerItem.h"

#include "Components/TextBlock.h"

void UUI_RankPlayerItem::SetupRankItem(int32 Rank, const FGsRankPlayerRecord& Record, bool bIsCurrentPlayer)
{
	bIsCurrentPlayerItem = bIsCurrentPlayer;

	if (RankText)
	{
		RankText->SetText(FText::FromString(FString::Printf(TEXT("#%d"), Rank)));
	}

	if (NameText)
	{
		NameText->SetText(FText::FromString(Record.PlayerName));
	}

	if (TimeText)
	{
		TimeText->SetText(FormatElapsedTime(Record.ElapsedMilliseconds));
	}

	if (KillText)
	{
		KillText->SetText(FText::AsNumber(Record.KillCount));
	}

	if (ReasonText)
	{
		ReasonText->SetText(GetSettleReasonText(Record.SettleReason));
	}

	const float TargetScale = bIsCurrentPlayerItem ? CurrentPlayerScale : NormalPlayerScale;
	SetRenderScale(FVector2D(TargetScale, TargetScale));
}

FText UUI_RankPlayerItem::FormatElapsedTime(int32 ElapsedMilliseconds)
{
	const int32 SafeMilliseconds = FMath::Max(0, ElapsedMilliseconds);
	const int32 TotalSeconds = SafeMilliseconds / 1000;
	const int32 Minutes = TotalSeconds / 60;
	const int32 Seconds = TotalSeconds % 60;
	const int32 Milliseconds = SafeMilliseconds % 1000;

	return FText::FromString(FString::Printf(TEXT("%02d:%02d.%03d"), Minutes, Seconds, Milliseconds));
}

FText UUI_RankPlayerItem::GetSettleReasonText(EGsRankSettleReason Reason)
{
	switch (Reason)
	{
	case EGsRankSettleReason::Completed:
		return FText::FromString(TEXT("通关"));
	case EGsRankSettleReason::TimeOut:
		return FText::FromString(TEXT("超时"));
	case EGsRankSettleReason::Interrupted:
		return FText::FromString(TEXT("中断"));
	default:
		return FText::FromString(TEXT("未知"));
	}
}
