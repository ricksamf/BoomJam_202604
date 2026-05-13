// ===================================================
// 文件：MachineGunEnemy.h
// 说明：表世界机枪敌人。Warmup → Burst(扇形+跟踪) → Cooldown。
// ===================================================

#pragma once

#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "MachineGunEnemy.generated.h"

class USceneComponent;
class UMachineGunEnemyDataAsset;

UCLASS()
class UEGAMEJAM_API AMachineGunEnemy : public AEnemyCharacter
{
	GENERATED_BODY()

public:
	AMachineGunEnemy();

	/** 发射一发子弹（带扇形散射） */
	UFUNCTION(BlueprintCallable, Category="Enemy|MG")
	void FireOneBullet(const FVector& AimLocation);

	/** 播放 DataAsset 里配置的 BurstMontage（由 FireOneBullet 每颗子弹调用） */
	UFUNCTION(BlueprintCallable, Category="Enemy|MG")
	void PlayAttackMontage();

	/** 在枪口位置一次性 Spawn DataAsset 里的 WarningMuzzleFX（开火前预警特效） */
	UFUNCTION(BlueprintCallable, Category="Enemy|MG")
	void SpawnWarningFX();

	/** 允许/禁止连发期间缓慢跟踪玩家 */
	UFUNCTION(BlueprintCallable, Category="Enemy|MG")
	void SetTrackingEnabled(bool bEnabled, AActor* TrackTarget);

	UFUNCTION(BlueprintPure, Category="Enemy|MG")
	FVector GetMuzzleLocation() const;

protected:
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy|MG")
	TObjectPtr<USceneComponent> MuzzleComp;

	UPROPERTY()
	TWeakObjectPtr<AActor> CurrentTrackTarget;

	UPROPERTY()
	bool bTracking = false;
};
