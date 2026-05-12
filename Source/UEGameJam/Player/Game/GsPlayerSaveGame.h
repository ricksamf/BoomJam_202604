// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GsPlayerSaveGame.generated.h"

UCLASS()
class UEGAMEJAM_API UGsPlayerSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	static UGsPlayerSaveGame* LoadOrCreate();
	static bool Save(const UGsPlayerSaveGame* SaveGame);
	static bool SaveVolumes(float NewBGMVolume, float NewSFXVolume);

	void SetBGMVolume(float NewVolume);
	void SetSFXVolume(float NewVolume);
	float GetBGMVolume() const { return BGMVolume; }
	float GetSFXVolume() const { return SFXVolume; }

	UPROPERTY()
	float BGMVolume = 1.f;

	UPROPERTY()
	float SFXVolume = 1.f;

private:
	static constexpr int32 UserIndex = 0;
};
