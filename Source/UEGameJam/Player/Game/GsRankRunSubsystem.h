// Copyright

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Player/Game/GsRankSaveGame.h"
#include "GsRankRunSubsystem.generated.h"

class AEnemyCharacter;

/**
 * 本局排行榜运行态，跨关卡保存当前玩家、计时和击杀数据。
 */
UCLASS()
class UEGAMEJAM_API UGsRankRunSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 便捷访问器 */
	UFUNCTION(BlueprintPure, Category="Rank", meta=(WorldContext="WorldContextObject"))
	static UGsRankRunSubsystem* Get(const UObject* WorldContextObject);

	/** 开始一局新的排行榜记录，会清空上一局的临时运行态 */
	UFUNCTION(BlueprintCallable, Category="Rank")
	bool StartRun(const FString& PlayerName);

	/** 记录一次玩家击杀；会随玩家复活重刷的敌人暂存到当前段，不会重刷的敌人立即计入已提交击杀 */
	UFUNCTION(BlueprintCallable, Category="Rank")
	void RegisterPlayerKill(AEnemyCharacter* KilledEnemy);

	/** 提交当前段击杀，进入下一段 */
	UFUNCTION(BlueprintCallable, Category="Rank")
	void CommitCurrentSegmentKills(int32 CheckpointIndex);

	/** 清空当前段击杀，已提交击杀不受影响 */
	UFUNCTION(BlueprintCallable, Category="Rank")
	void ResetCurrentSegmentKills(int32 CheckpointIndex);

	/** 结算当前本局成绩，通关、超时和中断都会进入同一排行榜 */
	UFUNCTION(BlueprintCallable, Category="Rank")
	bool SettleRun(const UObject* WorldContextObject, EGsRankSettleReason Reason);

	/** 获取本局真实用时，不受暂停和时间膨胀影响 */
	UFUNCTION(BlueprintPure, Category="Rank")
	float GetElapsedRunTimeSeconds() const;

	/** 获取本局剩余倒计时，不受暂停和时间膨胀影响 */
	UFUNCTION(BlueprintPure, Category="Rank")
	float GetRemainingTimeSeconds() const;

	/** 获取当前有效击杀数，包含已提交击杀和当前段击杀 */
	UFUNCTION(BlueprintPure, Category="Rank")
	int32 GetCurrentKillCount() const { return CommittedKillCount + CurrentSegmentKillCount; }

	/** 当前本局是否已经结算，防止重复保存成绩 */
	UFUNCTION(BlueprintPure, Category="Rank")
	bool HasSettledRun() const { return bHasSettledRun; }

	/** 当前是否有从登录界面开始的有效本局运行态 */
	UFUNCTION(BlueprintPure, Category="Rank")
	bool HasActiveRun() const { return bHasActiveRun; }

	FString GetCurrentPlayerName() const { return CurrentPlayerName; }

private:
	float GetRankTimeLimitSeconds() const;

private:
	FString CurrentPlayerName;
	double RunStartRealSeconds = 0.0;
	int32 CommittedKillCount = 0;
	int32 CurrentSegmentKillCount = 0;
	float CurrentRankTimeLimitSeconds = 0.0f;
	bool bHasActiveRun = false;
	bool bHasSettledRun = false;
};
