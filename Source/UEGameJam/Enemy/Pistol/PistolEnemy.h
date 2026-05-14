// ===================================================
// 文件：PistolEnemy.h
// 说明：表世界手枪敌人。Aim → Fire → Cooldown 循环。
// ===================================================

#pragma once

#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "PistolEnemy.generated.h"

class UPistolEnemyDataAsset;
class AEnemyProjectile;
class UEnemyWeaponComponent;

UCLASS()
class UEGAMEJAM_API APistolEnemy : public AEnemyCharacter
{
	GENERATED_BODY()

public:
	APistolEnemy();

	virtual void ApplyDataAsset() override;

	/** 发射一发子弹朝 AimLocation 方向 */
	UFUNCTION(BlueprintCallable, Category="Enemy|Pistol")
	void FireProjectile(const FVector& AimLocation);

	/** 播放 DataAsset 里配置的 FireMontage（由 PistolAim Task 调用） */
	UFUNCTION(BlueprintCallable, Category="Enemy|Pistol")
	void PlayAttackMontage();

	/** 在枪口位置一次性 Spawn DataAsset 里的 WarningMuzzleFX（开火前预警特效） */
	UFUNCTION(BlueprintCallable, Category="Enemy|Pistol")
	void SpawnWarningFX();

	UFUNCTION(BlueprintPure, Category="Enemy|Pistol")
	FVector GetMuzzleLocation() const;

protected:
	/** Notify 接管开火:Aim 期间 Montage 触发 NotifyName="Fire" 时,从 AIController.CachedPlayer 拿位置 spawn 子弹 */
	virtual void HandleFireNotify() override;

	/** 武器组件,attach 到 SK 的 Weapon_Attach_R socket;mesh / 偏移 / 枪口 socket 由 DataAsset 配置 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy|Pistol")
	TObjectPtr<UEnemyWeaponComponent> Weapon;
};
