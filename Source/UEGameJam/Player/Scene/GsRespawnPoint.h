// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GsRespawnPoint.generated.h"

class USceneComponent;

/**
 * 可放置在场景中的玩家复活点。
 */
UCLASS(Blueprintable)
class UEGAMEJAM_API AGsRespawnPoint : public AActor
{
	GENERATED_BODY()

	/** 复活点根组件，使用 Actor 位置和朝向作为玩家复活 Transform */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SceneRoot;

protected:
	/** 复活点编号，越大表示越靠后的关卡进度 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Respawn Point", meta = (ClampMin = 0))
	int32 CheckpointIndex = 0;

	/** 每个死亡次数对应的复活提示文本，键为当前复活点累计死亡次数 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Respawn Point|Hint")
	TMap<int32, FText> DeathHintsByCount;

	/** 玩家在当前复活点累计死亡次数 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Respawn Point|Hint")
	int32 DeathCount = 0;

public:
	AGsRespawnPoint();

	UFUNCTION(BlueprintPure, Category="Respawn Point")
	int32 GetCheckpointIndex() const { return CheckpointIndex; }

	UFUNCTION(BlueprintCallable, Category="Respawn Point|Hint")
	int32 RegisterPlayerDeath();

	UFUNCTION(BlueprintPure, Category="Respawn Point|Hint")
	bool GetHintForDeathCount(int32 InDeathCount, FText& OutHintText) const;

	UFUNCTION(BlueprintPure, Category="Respawn Point|Hint")
	int32 GetDeathCount() const { return DeathCount; }
};
