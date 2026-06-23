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
		if (GetNormalizedRankPlayerName(PlayerRecord.PlayerName) == NormalizedName)
		{
			return true;
		}
	}

	return false;
}

bool UGsRankSaveGame::RegisterPlayerName(const FString& Name)
{
	const FString TrimmedName = Name.TrimStartAndEnd();
	if (TrimmedName.IsEmpty() || ContainsPlayerName(TrimmedName))
	{
		return false;
	}

	FGsRankPlayerRecord PlayerRecord;
	PlayerRecord.PlayerName = TrimmedName;
	PlayerRecords.Add(PlayerRecord);
	return true;
}
