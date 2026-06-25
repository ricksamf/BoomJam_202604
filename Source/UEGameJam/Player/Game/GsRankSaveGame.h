// Copyright

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GsRankSaveGame.generated.h"

UENUM(BlueprintType)
enum class EGsRankSettleReason : uint8
{
	None,
	Completed,
	TimeOut,
	Interrupted
};

USTRUCT(BlueprintType)
struct FGsRankPlayerRecord
{
	GENERATED_BODY()

	/** 玩家名字 */
	UPROPERTY()
	FString PlayerName;

	/** 本局结算击杀数 */
	UPROPERTY()
	int32 KillCount = 0;

	/** 本局用时，单位毫秒 */
	UPROPERTY()
	int32 ElapsedMilliseconds = 0;

	/** 本局结算原因 */
	UPROPERTY()
	EGsRankSettleReason SettleReason = EGsRankSettleReason::None;

	/** 提交顺序，越大表示越新的成绩 */
	UPROPERTY()
	int32 SubmissionOrder = 0;
};

UCLASS()
class UEGAMEJAM_API UGsRankSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	static UGsRankSaveGame* LoadOrCreate();
	static bool Save(const UGsRankSaveGame* SaveGame);

	bool ContainsPlayerName(const FString& Name) const;
	bool SubmitRankRecord(
		const FString& PlayerName,
		int32 KillCount,
		int32 ElapsedMilliseconds,
		EGsRankSettleReason SettleReason,
		int32& OutRank);

	UPROPERTY()
	TArray<FGsRankPlayerRecord> PlayerRecords;

	UPROPERTY()
	int32 NextSubmissionOrder = 1;

private:
	static constexpr int32 UserIndex = 0;
};
