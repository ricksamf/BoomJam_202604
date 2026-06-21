// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Scene/GsRespawnPoint.h"

#include "Components/SceneComponent.h"

AGsRespawnPoint::AGsRespawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
}

int32 AGsRespawnPoint::RegisterPlayerDeath()
{
	++DeathCount;
	return DeathCount;
}

bool AGsRespawnPoint::GetHintForDeathCount(int32 InDeathCount, FText& OutHintText) const
{
	const FText* HintText = DeathHintsByCount.Find(InDeathCount);
	if (!HintText)
	{
		OutHintText = FText::GetEmpty();
		return false;
	}

	OutHintText = *HintText;
	return !OutHintText.IsEmpty();
}
