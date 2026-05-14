// ===================================================
// 文件：EnemyWeaponComponent.cpp
// ===================================================

#include "EnemyWeaponComponent.h"

UEnemyWeaponComponent::UEnemyWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// 武器 mesh 不参与碰撞 / overlap,纯展示
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);

	// 接受光照投影,跟主角 mesh 一致
	SetCastShadow(true);
}

FVector UEnemyWeaponComponent::GetMuzzleLocation() const
{
	if (!MuzzleSocketName.IsNone() && DoesSocketExist(MuzzleSocketName))
	{
		return GetSocketLocation(MuzzleSocketName);
	}
	return GetComponentLocation() + GetComponentQuat().RotateVector(MuzzleLocalOffset);
}

FVector UEnemyWeaponComponent::GetMuzzleForward() const
{
	if (!MuzzleSocketName.IsNone() && DoesSocketExist(MuzzleSocketName))
	{
		return GetSocketRotation(MuzzleSocketName).Vector();
	}
	return GetForwardVector();
}
