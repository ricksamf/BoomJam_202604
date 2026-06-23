// Copyright

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GsRankSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FGsRankPlayerRecord
{
	GENERATED_BODY()

	/** 玩家名字，后续排名、分数等数据会和这条记录一起保存 */
	UPROPERTY()
	FString PlayerName;
};

UCLASS()
class UEGAMEJAM_API UGsRankSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	static UGsRankSaveGame* LoadOrCreate();
	static bool Save(const UGsRankSaveGame* SaveGame);

	bool ContainsPlayerName(const FString& Name) const;
	bool RegisterPlayerName(const FString& Name);

	UPROPERTY()
	TArray<FGsRankPlayerRecord> PlayerRecords;

private:
	static constexpr int32 UserIndex = 0;
};
