// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Game/GsPlayerSaveGame.h"

#include "Kismet/GameplayStatics.h"

static const TCHAR* GetPlayerSettingsSlotName()
{
	return TEXT("PlayerSettings");
}

UGsPlayerSaveGame* UGsPlayerSaveGame::LoadOrCreate()
{
	USaveGame* LoadedSaveGame = UGameplayStatics::LoadGameFromSlot(GetPlayerSettingsSlotName(), UserIndex);
	if (UGsPlayerSaveGame* PlayerSaveGame = Cast<UGsPlayerSaveGame>(LoadedSaveGame))
	{
		return PlayerSaveGame;
	}

	return Cast<UGsPlayerSaveGame>(UGameplayStatics::CreateSaveGameObject(StaticClass()));
}

bool UGsPlayerSaveGame::Save(const UGsPlayerSaveGame* SaveGame)
{
	if (!SaveGame)
	{
		return false;
	}

	return UGameplayStatics::SaveGameToSlot(const_cast<UGsPlayerSaveGame*>(SaveGame), GetPlayerSettingsSlotName(), UserIndex);
}

bool UGsPlayerSaveGame::SaveVolumes(float NewBGMVolume, float NewSFXVolume)
{
	UGsPlayerSaveGame* SaveGame = LoadOrCreate();
	if (!SaveGame)
	{
		return false;
	}

	SaveGame->SetBGMVolume(NewBGMVolume);
	SaveGame->SetSFXVolume(NewSFXVolume);
	return Save(SaveGame);
}

void UGsPlayerSaveGame::SetBGMVolume(float NewVolume)
{
	BGMVolume = FMath::Clamp(NewVolume, 0.f, 1.f);
}

void UGsPlayerSaveGame::SetSFXVolume(float NewVolume)
{
	SFXVolume = FMath::Clamp(NewVolume, 0.f, 1.f);
}
