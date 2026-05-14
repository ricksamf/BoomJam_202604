// ===================================================
// 文件：MachineGunEnemyDataAsset.h
// 说明：表世界机枪敌人数值配置。
// ===================================================

#pragma once

#include "CoreMinimal.h"
#include "EnemyDataAsset.h"
#include "MachineGunEnemyDataAsset.generated.h"

class AEnemyProjectile;
class UNiagaraSystem;
class UAnimMontage;
class UStaticMesh;
class USoundBase;

UCLASS(BlueprintType)
class UEGAMEJAM_API UMachineGunEnemyDataAsset : public UEnemyDataAsset
{
	GENERATED_BODY()

public:
	/** 预警阶段时长（1.0-1.5s） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MG", meta=(ClampMin=0))
	float WarmupDuration = 1.25f;

	/** 连发阶段时长（1.2-1.5s） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MG", meta=(ClampMin=0))
	float BurstDuration = 1.35f;

	/** 冷却阶段时长（1.5-2.0s） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MG", meta=(ClampMin=0))
	float Cooldown = 1.7f;

	/** 射速：每秒发射数（5-8） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MG", meta=(ClampMin=1))
	float FireRate = 6.f;

	/** 扇形散射半角（度） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MG", meta=(ClampMin=0))
	float SpreadHalfAngleDeg = 4.f;

	/** 连发期间缓慢跟踪玩家的偏航速率（度/秒） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MG", meta=(ClampMin=0))
	float TrackingYawSpeed = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MG", meta=(ClampMin=0))
	float BulletDamage = 6.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MG", meta=(ClampMin=0))
	float BulletSpeed = 5500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MG")
	TSubclassOf<AEnemyProjectile> ProjectileClass;

	/** 开火闪光 Niagara */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MG|FX")
	TObjectPtr<UNiagaraSystem> MuzzleFlashFX;

	/** 开火前的枪口预警 Niagara(Warmup 剩余时间到 WarningLeadTime 时一次性在枪口 Spawn) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MG|FX")
	TObjectPtr<UNiagaraSystem> WarningMuzzleFX;

	/** 开火动画 Montage（每次 FireOneBullet 时播一次；留空即不播） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MG|Anim")
	TObjectPtr<UAnimMontage> BurstMontage;

	/** 开火音效列表（每颗子弹随机抽一个播放；留空即不播） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MG|Audio")
	TArray<TObjectPtr<USoundBase>> FireSounds;

	/** 武器 mesh,会被 attach 到 SK 的 Weapon_Attach_R socket(留空则手上无武器) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MG|Weapon")
	TObjectPtr<UStaticMesh> WeaponMesh;

	/** 武器 attach 后的本地变换微调(socket 上枪角度/位置不对时用) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MG|Weapon")
	FTransform WeaponAttachTransform;

	/** 武器 mesh 上的枪口 socket 名,不存在时退回 WeaponMuzzleLocalOffset */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MG|Weapon")
	FName WeaponMuzzleSocket = FName("Muzzle");

	/** Muzzle socket 不存在时的本地偏移(组件本地空间,常见 X 前向) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MG|Weapon")
	FVector WeaponMuzzleLocalOffset = FVector(50.f, 0.f, 0.f);
};
