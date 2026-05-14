// ===================================================
// 文件：EnemyRespawnSubsystem.h
// 说明：敌人重生管理器。关卡里手放的 AEnemyCharacter 在 BeginPlay
//       时把自身 (Class, Transform, OwningCheckpoint) 快照成一条
//       Record；Die() 时把对应 Record 标记为 bIsDead；玩家
//       OnRespawn 时遍历 bIsDead 的 Record 全部重 spawn。
//       重 spawn 通过 SpawnActorDeferred 把 RecordId 预设到新实例
//       上，避免新实例的 BeginPlay 重复登记。
// ===================================================

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EnemyRespawnSubsystem.generated.h"

class AEnemyCharacter;

USTRUCT()
struct FEnemyRespawnRecord
{
	GENERATED_BODY()

	UPROPERTY()
	TSubclassOf<AEnemyCharacter> EnemyClass;

	UPROPERTY()
	FTransform SpawnTransform = FTransform::Identity;

	/** -1 = 自动按最近 RespawnPoint 推断；>=0 = 怪物上手动覆盖 */
	UPROPERTY()
	int32 OwningCheckpoint = -1;

	/** 当前是否处于死亡待重生状态 */
	UPROPERTY()
	bool bIsDead = false;

	/** 调试用：原 actor 名 */
	UPROPERTY()
	FName SourceName;
};

UCLASS()
class UEGAMEJAM_API UEnemyRespawnSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	/** 便捷访问器 */
	UFUNCTION(BlueprintPure, Category="Enemy|Respawn", meta=(WorldContext="WorldContext"))
	static UEnemyRespawnSubsystem* Get(const UObject* WorldContext);

	/** 怪物 BeginPlay 调用。InOutRecordId 已 >=0 表示是重生流程派生的实例，
	 *  此时仅把对应记录 bIsDead 清掉、不增加新记录；否则创建新记录并把 ID 写回。 */
	void RegisterEnemy(AEnemyCharacter* Enemy, int32& InOutRecordId);

	/** 怪物 Die 调用。 */
	void MarkDead(int32 RecordId);

	/** 把所有 bIsDead 的记录重新 spawn。玩家 OnRespawn 触发；也可手动调试。
	 *  当前 A 模式：所有死怪一次性全复活。 */
	UFUNCTION(BlueprintCallable, Category="Enemy|Respawn")
	void RespawnAllDead();

	/** 调试：打印当前 Record 列表 */
	UFUNCTION(Exec, Category="Enemy|Respawn")
	void EnemyRespawnDump() const;

private:
	UPROPERTY()
	TArray<FEnemyRespawnRecord> Records;

	UFUNCTION()
	void HandlePlayerRespawn();

	int32 InferOwningCheckpoint(const FTransform& Where) const;
};
