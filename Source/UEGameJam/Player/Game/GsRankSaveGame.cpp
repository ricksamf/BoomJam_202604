// Copyright

#include "Player/Game/GsRankSaveGame.h"

#include "Kismet/GameplayStatics.h"

static const TCHAR* GetRankRecordsSlotName()
{
	return TEXT("RankRecords");
}

static FString GetNormalizedRankPlayerName(const FString& Name)
{
	return Name.TrimStartAndEnd().ToLower();
}

static bool IsLegacyPlaceholderRecord(const FGsRankPlayerRecord& PlayerRecord)
{
	return PlayerRecord.SettleReason == EGsRankSettleReason::None
		&& PlayerRecord.KillCount == 0
		&& PlayerRecord.ElapsedMilliseconds == 0
		&& PlayerRecord.SubmissionOrder == 0;
}

UGsRankSaveGame* UGsRankSaveGame::LoadOrCreate()
{
	USaveGame* LoadedSaveGame = UGameplayStatics::LoadGameFromSlot(GetRankRecordsSlotName(), UserIndex);
	if (UGsRankSaveGame* RankSaveGame = Cast<UGsRankSaveGame>(LoadedSaveGame))
	{
		return RankSaveGame;
	}

	return Cast<UGsRankSaveGame>(UGameplayStatics::CreateSaveGameObject(StaticClass()));
}

bool UGsRankSaveGame::Save(const UGsRankSaveGame* SaveGame)
{
	if (!SaveGame)
	{
		return false;
	}

	return UGameplayStatics::SaveGameToSlot(const_cast<UGsRankSaveGame*>(SaveGame), GetRankRecordsSlotName(), UserIndex);
}

bool UGsRankSaveGame::ContainsPlayerName(const FString& Name) const
{
	const FString NormalizedName = GetNormalizedRankPlayerName(Name);
	if (NormalizedName.IsEmpty())
	{
		return false;
	}

	for (const FGsRankPlayerRecord& PlayerRecord : PlayerRecords)
	{
		if (IsLegacyPlaceholderRecord(PlayerRecord))
		{
			continue;
		}

		if (GetNormalizedRankPlayerName(PlayerRecord.PlayerName) == NormalizedName)
		{
			return true;
		}
	}

	return false;
}

bool UGsRankSaveGame::SubmitRankRecord(
	const FString& PlayerName,
	int32 KillCount,
	int32 ElapsedMilliseconds,
	EGsRankSettleReason SettleReason,
	int32& OutRank)
{
	OutRank = INDEX_NONE;

	const FString TrimmedName = PlayerName.TrimStartAndEnd();
	if (TrimmedName.IsEmpty())
	{
		return false;
	}

	FGsRankPlayerRecord* TargetRecord = nullptr;
	const FString NormalizedName = GetNormalizedRankPlayerName(TrimmedName);
	PlayerRecords.RemoveAll([&NormalizedName](const FGsRankPlayerRecord& PlayerRecord)
	{
		return IsLegacyPlaceholderRecord(PlayerRecord)
			&& GetNormalizedRankPlayerName(PlayerRecord.PlayerName) != NormalizedName;
	});

	for (FGsRankPlayerRecord& PlayerRecord : PlayerRecords)
	{
		if (GetNormalizedRankPlayerName(PlayerRecord.PlayerName) == NormalizedName)
		{
			TargetRecord = &PlayerRecord;
			break;
		}
	}

	if (!TargetRecord)
	{
		FGsRankPlayerRecord PlayerRecord;
		PlayerRecord.PlayerName = TrimmedName;
		const int32 AddedIndex = PlayerRecords.Add(PlayerRecord);
		TargetRecord = &PlayerRecords[AddedIndex];
	}

	TargetRecord->PlayerName = TrimmedName;
	TargetRecord->KillCount = FMath::Max(0, KillCount);
	TargetRecord->ElapsedMilliseconds = FMath::Max(0, ElapsedMilliseconds);
	TargetRecord->SettleReason = SettleReason;
	TargetRecord->SubmissionOrder = NextSubmissionOrder++;

	PlayerRecords.RemoveAll([](const FGsRankPlayerRecord& PlayerRecord)
	{
		return IsLegacyPlaceholderRecord(PlayerRecord);
	});

	PlayerRecords.Sort([](const FGsRankPlayerRecord& Left, const FGsRankPlayerRecord& Right)
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
	});

	for (int32 Index = 0; Index < PlayerRecords.Num(); ++Index)
	{
		const FGsRankPlayerRecord& PlayerRecord = PlayerRecords[Index];
		if (GetNormalizedRankPlayerName(PlayerRecord.PlayerName) == NormalizedName)
		{
			OutRank = Index + 1;
			break;
		}
	}

	return OutRank != INDEX_NONE;
}
