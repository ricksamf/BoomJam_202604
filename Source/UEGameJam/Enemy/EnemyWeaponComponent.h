// ===================================================
// 文件：EnemyWeaponComponent.h
// 说明：远程敌人(Pistol / MachineGun)挂在手部 socket 上的武器组件。
//       继承 UStaticMeshComponent;mesh 和 attach 偏移由各自 DataAsset 配置;
//       提供 GetMuzzleLocation / GetMuzzleForward,内部优先用武器 mesh 自身的
//       Muzzle socket,没有则用 ComponentLocation + 本地偏移作 fallback。
// ===================================================

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "EnemyWeaponComponent.generated.h"

UCLASS(ClassGroup=(Enemy), meta=(BlueprintSpawnableComponent))
class UEGAMEJAM_API UEnemyWeaponComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	UEnemyWeaponComponent();

	/** 武器 mesh 上的"枪口" socket 名;不存在时用 MuzzleLocalOffset 作 fallback */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|Weapon")
	FName MuzzleSocketName = FName("Muzzle");

	/** 武器 mesh 没 Muzzle socket 时,在组件本地空间下作为枪口的偏移(常见 X 前向 ~50cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|Weapon")
	FVector MuzzleLocalOffset = FVector(50.f, 0.f, 0.f);

	/** 枪口世界位置:有 Muzzle socket 用 socket,否则 ComponentLocation + Quat * Offset */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Enemy|Weapon")
	FVector GetMuzzleLocation() const;

	/** 枪口世界 forward 方向:有 socket 用 socket 朝向,否则用组件 forward */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Enemy|Weapon")
	FVector GetMuzzleForward() const;
};
