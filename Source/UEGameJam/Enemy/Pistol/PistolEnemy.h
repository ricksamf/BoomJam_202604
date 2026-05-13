// ===================================================
// 文件：PistolEnemy.h
// 说明：表世界手枪敌人。Aim → Fire → Cooldown 循环。
// ===================================================

#pragma once

#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "PistolEnemy.generated.h"

class USceneComponent;
class UPistolEnemyDataAsset;
class AEnemyProjectile;

UCLASS()
class UEGAMEJAM_API APistolEnemy : public AEnemyCharacter
{
	GENERATED_BODY()

public:
	APistolEnemy();

	/** 发射一发子弹朝 AimLocation 方向 */
	UFUNCTION(BlueprintCallable, Category="Enemy|Pistol")
	void FireProjectile(const FVector& AimLocation);

	/** 播放 DataAsset 里配置的 FireMontage（由 FireProjectile 调用） */
	UFUNCTION(BlueprintCallable, Category="Enemy|Pistol")
	void PlayAttackMontage();

	/** 在枪口位置一次性 Spawn DataAsset 里的 WarningMuzzleFX（开火前预警特效） */
	UFUNCTION(BlueprintCallable, Category="Enemy|Pistol")
	void SpawnWarningFX();

	UFUNCTION(BlueprintPure, Category="Enemy|Pistol")
	FVector GetMuzzleLocation() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy|Pistol")
	TObjectPtr<USceneComponent> MuzzleComp;
};
