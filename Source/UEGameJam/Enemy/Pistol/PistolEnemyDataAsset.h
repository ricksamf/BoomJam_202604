// ===================================================
// 文件：PistolEnemyDataAsset.h
// 说明：表世界手枪敌人数值配置。
// ===================================================

#pragma once

#include "CoreMinimal.h"
#include "EnemyDataAsset.h"
#include "PistolEnemyDataAsset.generated.h"

class AEnemyProjectile;
class UNiagaraSystem;
class UAnimMontage;
class UStaticMesh;
class USoundBase;

UCLASS(BlueprintType)
class UEGAMEJAM_API UPistolEnemyDataAsset : public UEnemyDataAsset
{
	GENERATED_BODY()

public:
	/** 瞄准阶段时长（策划案 0.8-1.2s） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pistol", meta=(ClampMin=0))
	float AimDuration = 1.f;

	/** 冷却阶段时长（策划案 1.5-1.8s） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pistol", meta=(ClampMin=0))
	float Cooldown = 1.6f;

	/** 子弹速度 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pistol", meta=(ClampMin=0))
	float ProjectileSpeed = 4000.f;

	/** 子弹伤害 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pistol", meta=(ClampMin=0))
	float ProjectileDamage = 15.f;

	/** 子弹类 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pistol")
	TSubclassOf<AEnemyProjectile> ProjectileClass;

	/** 开火闪光 Niagara */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pistol|FX")
	TObjectPtr<UNiagaraSystem> MuzzleFlashFX;

	/** 开火前的枪口预警 Niagara(Aim 剩余时间到 WarningLeadTime 时一次性在枪口 Spawn) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pistol|FX")
	TObjectPtr<UNiagaraSystem> WarningMuzzleFX;

	/** 开火动画 Montage（在 FireProjectile 时播一次，留空即不播） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pistol|Anim")
	TObjectPtr<UAnimMontage> FireMontage;

	/** 开火音效列表（每次开火随机抽一个播放；留空即不播） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pistol|Audio")
	TArray<TObjectPtr<USoundBase>> FireSounds;

	/** 武器 mesh,会被 attach 到 SK 的 Weapon_Attach_R socket(留空则手上无武器) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pistol|Weapon")
	TObjectPtr<UStaticMesh> WeaponMesh;

	/** 武器 attach 后的本地变换微调(socket 上枪角度/位置不对时用) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pistol|Weapon")
	FTransform WeaponAttachTransform;

	/** 武器 mesh 上的枪口 socket 名,不存在时退回 WeaponMuzzleLocalOffset */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pistol|Weapon")
	FName WeaponMuzzleSocket = FName("Muzzle");

	/** Muzzle socket 不存在时的本地偏移(组件本地空间,常见 X 前向) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pistol|Weapon")
	FVector WeaponMuzzleLocalOffset = FVector(50.f, 0.f, 0.f);
};
