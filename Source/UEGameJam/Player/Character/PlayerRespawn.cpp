// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Character/GsPlayer.h"

#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Character/GsPlayerResourceDataAsset.h"
#include "Player/Game/GsLevelStateGameState.h"
#include "TimerManager.h"

void AGsPlayer::Die()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	bIsWaitingForRespawnInput = true;
	bIsDeathTimeDilationActive = true;
	DeathTimeDilationElapsed = 0.0f;
	bResetFirstPersonCameraLocationOnNextUpdate = true;
	FinishMeleeHitStop();

	if (PlayerResourceData && PlayerResourceData->DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PlayerResourceData->DeathSound, GetActorLocation());
	}
	UGameplayStatics::SetGlobalTimeDilation(this, 1.0f);

	StopSlide(true);
	if (IsDashing())
	{
		AbortDash();
	}
	AbortGrapple();
	if (IsLedgeClimbing())
	{
		AbortLedgeClimb();
	}
	if (IsWallRunning())
	{
		StopWallRun();
	}
	else
	{
		ClearDashState();
		FinishCharacterAction();
	}
	bHasDashedSinceLanded = false;
	ClearGrappleState();
	ResetWallRunDetection();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MeleeHitTimer);
		World->GetTimerManager().ClearTimer(MeleeHitStopTimer);
	}

	if (UCharacterMovementComponent* PlayerMovementComponent = GetCharacterMovement())
	{
		PlayerMovementComponent->StopMovementImmediately();
		PlayerMovementComponent->StopActiveMovement();
		PlayerMovementComponent->DisableMovement();
	}

	OnDamaged.Broadcast(0.0f);
	OnDeath.Broadcast();
	BP_OnDeath();
}

void AGsPlayer::RespawnFromCheckpoint()
{
	FTransform RespawnTransform = bHasSafeLocation
		? FTransform(LastSafeRotation, LastSafeLocation)
		: GetActorTransform();

	if (UWorld* World = GetWorld())
	{
		if (AGsLevelStateGameState* LevelState = World->GetGameState<AGsLevelStateGameState>())
		{
			LevelState->EnsureFallbackRespawnTransform(RespawnTransform);
			LevelState->GetCurrentRespawnTransform(RespawnTransform);
		}
	}

	ResetForRespawn(RespawnTransform);
}

void AGsPlayer::ResetForRespawn(const FTransform& RespawnTransform)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MeleeHitTimer);
		World->GetTimerManager().ClearTimer(MeleeHitStopTimer);
		World->GetTimerManager().ClearTimer(ActionTimer);
	}
	FinishMeleeHitStop();

	const FGsPlayerTuningRow& PlayerTuning = GetPlayerTuning();
	CurrentHP = PlayerTuning.MaxHP;
	bIsDead = false;
	bIsWaitingForRespawnInput = false;
	bIsDeathTimeDilationActive = false;
	DeathTimeDilationElapsed = 0.0f;
	CurrentAction = EUEGameJamPlayerAction::None;
	CachedMoveInput = FVector2D::ZeroVector;
	bIsSlideInputHeld = false;
	bIsWaitingToStopSlideWhenCanStand = false;
	CurrentSlideSpeed = 0.0f;
	SlideDirection = FVector::ForwardVector;
	bHasDashedSinceLanded = false;
	bIsFalculaLaunching = false;
	bIsRecoveringFromFall = false;
	bHasSafeLocation = true;
	LastSafeLocation = RespawnTransform.GetLocation();
	LastSafeRotation = RespawnTransform.GetRotation().Rotator();
	bResetFirstPersonCameraLocationOnNextUpdate = true;
	TargetWallRunCameraRoll = 0.0f;
	CurrentWallRunCameraRoll = 0.0f;
	SetWallRunCameraTiltTarget(0.0f);
	ClearDashState();
	ClearGrappleState();
	ClearLedgeClimbState();
	ResetWallRunDetection();

	if (UWorld* World = GetWorld())
	{
		const float CurrentWorldTime = World->GetTimeSeconds();
		LastDashTime = CurrentWorldTime - PlayerTuning.DashCooldown;
		LastFalculaTime = CurrentWorldTime - PlayerTuning.GrappleCooldown;
		LastSkillCastTime = CurrentWorldTime - PlayerTuning.SkillCooldown;
		LastFallRecoveryTime = CurrentWorldTime - PlayerTuning.SafeLandingMinInterval;
	}
	else
	{
		LastDashTime = -PlayerTuning.DashCooldown;
		LastFalculaTime = -PlayerTuning.GrappleCooldown;
		LastSkillCastTime = -PlayerTuning.SkillCooldown;
		LastFallRecoveryTime = -PlayerTuning.SafeLandingMinInterval;
	}

	const FRotator RespawnRotation = RespawnTransform.GetRotation().Rotator();
	SetActorLocationAndRotation(RespawnTransform.GetLocation(), RespawnRotation, false, nullptr, ETeleportType::TeleportPhysics);

	if (UCharacterMovementComponent* PlayerMovementComponent = GetCharacterMovement())
	{
		PlayerMovementComponent->StopMovementImmediately();
		PlayerMovementComponent->StopActiveMovement();
		PlayerMovementComponent->Velocity = FVector::ZeroVector;
		PlayerMovementComponent->SetMovementMode(MOVE_Walking);
	}

	if (AController* PlayerController = GetController())
	{
		PlayerController->SetControlRotation(RespawnRotation);
	}

	UGameplayStatics::SetGlobalTimeDilation(this, 1.0f);
	OnDamaged.Broadcast(GetLifePercent());
	OnRespawn.Broadcast();
}
