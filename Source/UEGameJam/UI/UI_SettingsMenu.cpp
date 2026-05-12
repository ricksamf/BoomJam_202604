// Fill out your copyright notice in the Description page of Project Settings.

#include "UI_SettingsMenu.h"

#include "Audio/BgmSubsystem.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Player/Game/GsPlayerSaveGame.h"

void UUI_SettingsMenu::NativeConstruct()
{
	Super::NativeConstruct();

	float BGMVolume = 1.f;
	float SFXVolume = 1.f;
	if (UGsPlayerSaveGame* SaveGame = UGsPlayerSaveGame::LoadOrCreate())
	{
		BGMVolume = SaveGame->GetBGMVolume();
		SFXVolume = SaveGame->GetSFXVolume();
	}

	if (UBgmSubsystem* BgmSubsystem = UBgmSubsystem::Get(this))
	{
		BgmSubsystem->SetBGMVolume(BGMVolume);
		BgmSubsystem->SetSFXVolume(SFXVolume);
	}

	if (BGMSlider)
	{
		BGMSlider->OnValueChanged.RemoveDynamic(this, &UUI_SettingsMenu::HandleBGMVolumeChanged);
		BGMSlider->SetValue(BGMVolume);
		BGMSlider->OnValueChanged.AddDynamic(this, &UUI_SettingsMenu::HandleBGMVolumeChanged);
	}

	if (SFXSlider)
	{
		SFXSlider->OnValueChanged.RemoveDynamic(this, &UUI_SettingsMenu::HandleSFXVolumeChanged);
		SFXSlider->SetValue(SFXVolume);
		SFXSlider->OnValueChanged.AddDynamic(this, &UUI_SettingsMenu::HandleSFXVolumeChanged);
	}

	if (BackBtn)
	{
		BackBtn->OnClicked.RemoveDynamic(this, &UUI_SettingsMenu::HandleBackClicked);
		BackBtn->OnClicked.AddDynamic(this, &UUI_SettingsMenu::HandleBackClicked);
	}
}

void UUI_SettingsMenu::HandleBGMVolumeChanged(float Value)
{
	if (UBgmSubsystem* BgmSubsystem = UBgmSubsystem::Get(this))
	{
		BgmSubsystem->SetBGMVolume(Value);
	}

	SaveCurrentVolumes();
}

void UUI_SettingsMenu::HandleSFXVolumeChanged(float Value)
{
	if (UBgmSubsystem* BgmSubsystem = UBgmSubsystem::Get(this))
	{
		BgmSubsystem->SetSFXVolume(Value);
	}

	SaveCurrentVolumes();
}

void UUI_SettingsMenu::HandleBackClicked()
{
	RemoveFromParent();
}

void UUI_SettingsMenu::SaveCurrentVolumes() const
{
	const float BGMVolume = BGMSlider ? BGMSlider->GetValue() : 1.f;
	const float SFXVolume = SFXSlider ? SFXSlider->GetValue() : 1.f;
	UGsPlayerSaveGame::SaveVolumes(BGMVolume, SFXVolume);
}
