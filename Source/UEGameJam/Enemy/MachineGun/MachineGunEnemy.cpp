// ===================================================
// 文件：MachineGunEnemy.cpp
// ===================================================

#include "MachineGunEnemy.h"
#include "MachineGunEnemyDataAsset.h"
#include "EnemyAIController.h"
#include "EnemyProjectile.h"
#include "Animation/AnimMontage.h"
#include "Components/SceneComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

AMachineGunEnemy::AMachineGunEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	DefaultRealmType = ERealmType::Surface;

	MuzzleComp = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle"));
	MuzzleComp->SetupAttachment(GetMesh());
}

void AMachineGunEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 连发期间缓慢跟踪
	if (bTracking)
	{
		AActor* T = CurrentTrackTarget.Get();
		if (T && !IsDead())
		{
			FVector ToTarget = T->GetActorLocation() - GetActorLocation();
			ToTarget.Z = 0.f;
			if (!ToTarget.IsNearlyZero())
			{
				const FRotator Desired = ToTarget.Rotation();
				float YawRate = 45.f;
				if (const UMachineGunEnemyDataAsset* Data = Cast<UMachineGunEnemyDataAsset>(EnemyData))
				{
					YawRate = Data->TrackingYawSpeed;
				}
				const FRotator Cur = GetActorRotation();
				FRotator New = FMath::RInterpConstantTo(Cur, FRotator(0.f, Desired.Yaw, 0.f), DeltaSeconds, YawRate);
				New.Pitch = 0.f;
				New.Roll = 0.f;
				SetActorRotation(New);
			}
		}
	}
}

void AMachineGunEnemy::SetTrackingEnabled(bool bEnabled, AActor* TrackTarget)
{
	bTracking = bEnabled;
	CurrentTrackTarget = TrackTarget;
}

void AMachineGunEnemy::FireOneBullet(const FVector& AimLocation)
{
	if (IsDead() || !EnemyData)
	{
		return;
	}
	const UMachineGunEnemyDataAsset* Data = Cast<UMachineGunEnemyDataAsset>(EnemyData);
	if (!Data || !Data->ProjectileClass)
	{
		return;
	}

	const FVector SpawnLoc = GetMuzzleLocation();
	FVector BaseDir = AimLocation - SpawnLoc;
	if (BaseDir.IsNearlyZero())
	{
		BaseDir = GetActorForwardVector();
	}
	BaseDir = BaseDir.GetSafeNormal();

	// 扇形散射
	const float HalfAngleRad = FMath::DegreesToRadians(Data->SpreadHalfAngleDeg);
	const FVector Dir = FMath::VRandCone(BaseDir, HalfAngleRad);

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AEnemyProjectile* Proj = GetWorld()->SpawnActor<AEnemyProjectile>(Data->ProjectileClass, SpawnLoc, Dir.Rotation(), Params);
	if (Proj)
	{
		Proj->InitializeAndLaunch(Dir, Data->BulletSpeed, this, GetEnemyRealmType());
	}

	if (Data->MuzzleFlashFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, Data->MuzzleFlashFX, SpawnLoc, Dir.Rotation());
	}

	PlayRandomSound(Data->FireSounds, SpawnLoc);
}

FVector AMachineGunEnemy::GetMuzzleLocation() const
{
	return MuzzleComp ? MuzzleComp->GetComponentLocation() : GetActorLocation();
}

void AMachineGunEnemy::PlayAttackMontage()
{
	if (IsDead())
	{
		return;
	}
	const UMachineGunEnemyDataAsset* Data = Cast<UMachineGunEnemyDataAsset>(EnemyData);
	if (!Data || !Data->BurstMontage)
	{
		return;
	}
	// Burst 期间一次性触发,Montage 资产里设置 Section 循环维持节奏。
	// 子弹由 Montage "Fire" AnimNotify 驱动 spawn(见 HandleFireNotify)。
	PlayAnimMontage(Data->BurstMontage);
}

void AMachineGunEnemy::StopAttackMontage()
{
	const UMachineGunEnemyDataAsset* Data = Cast<UMachineGunEnemyDataAsset>(EnemyData);
	if (!Data || !Data->BurstMontage)
	{
		return;
	}
	// Burst 总时长到了,停 Montage(走 Montage 资产的 BlendOutTime)
	StopAnimMontage(Data->BurstMontage);
}

void AMachineGunEnemy::HandleFireNotify()
{
	if (IsDead())
	{
		return;
	}

	AActor* Target = CurrentTrackTarget.Get();
	if (!Target)
	{
		if (AEnemyAIController* AIC = Cast<AEnemyAIController>(GetController()))
		{
			Target = AIC->GetCachedPlayer();
		}
	}

	const FVector AimLoc = Target
		? Target->GetActorLocation()
		: GetMuzzleLocation() + GetActorForwardVector() * 10000.f;

	FireOneBullet(AimLoc);
}

void AMachineGunEnemy::SpawnWarningFX()
{
	if (IsDead() || !EnemyData)
	{
		return;
	}
	const UMachineGunEnemyDataAsset* Data = Cast<UMachineGunEnemyDataAsset>(EnemyData);
	if (!Data || !Data->WarningMuzzleFX)
	{
		return;
	}
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, Data->WarningMuzzleFX,
		GetMuzzleLocation(), GetActorForwardVector().Rotation());
}
