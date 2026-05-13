// ===================================================
// 文件：PistolEnemy.cpp
// ===================================================

#include "PistolEnemy.h"
#include "PistolEnemyDataAsset.h"
#include "EnemyProjectile.h"
#include "Animation/AnimMontage.h"
#include "Components/SceneComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"

APistolEnemy::APistolEnemy()
{
	PrimaryActorTick.bCanEverTick = false;

	DefaultRealmType = ERealmType::Surface;

	MuzzleComp = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle"));
	MuzzleComp->SetupAttachment(GetMesh());
}

void APistolEnemy::FireProjectile(const FVector& AimLocation)
{
	if (IsDead() || !EnemyData)
	{
		return;
	}
	const UPistolEnemyDataAsset* Data = Cast<UPistolEnemyDataAsset>(EnemyData);
	if (!Data || !Data->ProjectileClass)
	{
		return;
	}

	PlayAttackMontage();

	const FVector SpawnLoc = GetMuzzleLocation();
	FVector Dir = AimLocation - SpawnLoc;
	if (Dir.IsNearlyZero())
	{
		Dir = GetActorForwardVector();
	}
	Dir = Dir.GetSafeNormal();

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AEnemyProjectile* Proj = GetWorld()->SpawnActor<AEnemyProjectile>(Data->ProjectileClass, SpawnLoc, Dir.Rotation(), Params);
	if (Proj)
	{
		Proj->InitializeAndLaunch(Dir, Data->ProjectileSpeed, this, GetEnemyRealmType());
	}

	if (Data->MuzzleFlashFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, Data->MuzzleFlashFX, SpawnLoc, Dir.Rotation());
	}

	PlayRandomSound(Data->FireSounds, SpawnLoc);
}

FVector APistolEnemy::GetMuzzleLocation() const
{
	return MuzzleComp ? MuzzleComp->GetComponentLocation() : GetActorLocation();
}

void APistolEnemy::PlayAttackMontage()
{
	if (IsDead())
	{
		return;
	}
	const UPistolEnemyDataAsset* Data = Cast<UPistolEnemyDataAsset>(EnemyData);
	if (!Data || !Data->FireMontage)
	{
		return;
	}
	PlayAnimMontage(Data->FireMontage);
}

void APistolEnemy::SpawnWarningFX()
{
	if (IsDead() || !EnemyData)
	{
		return;
	}
	const UPistolEnemyDataAsset* Data = Cast<UPistolEnemyDataAsset>(EnemyData);
	if (!Data || !Data->WarningMuzzleFX)
	{
		return;
	}
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, Data->WarningMuzzleFX,
		GetMuzzleLocation(), GetActorForwardVector().Rotation());
}
