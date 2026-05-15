// Fill out your copyright notice in the Description page of Project Settings.

#include "UI_SettingsMenu.h"

#include "Audio/BgmSubsystem.h"
#include "Components/Button.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/GameUserSettings.h"
#include "Player/Game/GsPlayerSaveGame.h"
#include "UI_SettingsWidget.h"

static FIntPoint GetFallbackWindowModeResolution(const UGameUserSettings* GameUserSettings, EWindowMode::Type WindowMode)
{
	if (!GameUserSettings)
	{
		return FIntPoint(1280, 720);
	}

	if (WindowMode == EWindowMode::WindowedFullscreen || WindowMode == EWindowMode::Fullscreen)
	{
		const FIntPoint DesktopResolution = GameUserSettings->GetDesktopResolution();
		if (DesktopResolution.X > 0 && DesktopResolution.Y > 0)
		{
			return DesktopResolution;
		}
	}

	return FIntPoint(1280, 720);
}

static FIntPoint GetValidWindowModeResolution(const UGameUserSettings* GameUserSettings, EWindowMode::Type WindowMode)
{
	if (!GameUserSettings)
	{
		return FIntPoint(1280, 720);
	}

	if (WindowMode == EWindowMode::WindowedFullscreen)
	{
		const FIntPoint DesktopResolution = GameUserSettings->GetDesktopResolution();
		if (DesktopResolution.X > 0 && DesktopResolution.Y > 0)
		{
			return DesktopResolution;
		}
	}

	FIntPoint Resolution = GameUserSettings->GetScreenResolution();
	if (Resolution.X > 0 && Resolution.Y > 0)
	{
		return Resolution;
	}

	if (GEngine && GEngine->GameViewport && GEngine->GameViewport->Viewport)
	{
		Resolution = GEngine->GameViewport->Viewport->GetSizeXY();
		if (Resolution.X > 0 && Resolution.Y > 0)
		{
			return Resolution;
		}
	}

	return GetFallbackWindowModeResolution(GameUserSettings, WindowMode);
}

static bool IsValidResolution(const FIntPoint& Resolution)
{
	return Resolution.X > 0 && Resolution.Y > 0;
}

static TArray<FIntPoint> GetDefaultResolutionOptions()
{
	TArray<FIntPoint> Options;
	Options.Emplace(1280, 720);
	Options.Emplace(1600, 900);
	Options.Emplace(1920, 1080);
	Options.Emplace(2560, 1440);
	Options.Emplace(3840, 2160);
	return Options;
}

static FText GetResolutionText(const FIntPoint& Resolution)
{
	return FText::FromString(FString::Printf(TEXT("%dx%d"), Resolution.X, Resolution.Y));
}

static int32 FindResolutionIndex(const TArray<FIntPoint>& Options, const FIntPoint& Resolution)
{
	for (int32 Index = 0; Index < Options.Num(); ++Index)
	{
		if (Options[Index] == Resolution)
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

static TArray<FText> GetResolutionTextOptions(const TArray<FIntPoint>& Options)
{
	TArray<FText> TextOptions;
	TextOptions.Reserve(Options.Num());
	for (const FIntPoint& Resolution : Options)
	{
		TextOptions.Add(GetResolutionText(Resolution));
	}

	return TextOptions;
}

static float GetFrameRateLimitByIndex(int32 NewIndex)
{
	switch (NewIndex)
	{
	case 0:
		return 30.f;
	case 1:
		return 60.f;
	case 2:
		return 120.f;
	case 3:
		return 144.f;
	default:
		return 0.f;
	}
}

static TArray<FText> GetVolumeTextOptions()
{
	return
	{
		FText::FromString(TEXT("0")),
		FText::FromString(TEXT("20")),
		FText::FromString(TEXT("40")),
		FText::FromString(TEXT("60")),
		FText::FromString(TEXT("80")),
		FText::FromString(TEXT("100"))
	};
}

static float GetVolumeByIndex(int32 NewIndex)
{
	return FMath::Clamp(static_cast<float>(NewIndex) * 0.2f, 0.f, 1.f);
}

static int32 GetVolumeIndex(float Volume)
{
	return FMath::Clamp(FMath::RoundToInt(Volume * 5.f), 0, 5);
}

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

	if (WindowModeSetting)
	{
		WindowModeSetting->OnSelectionChanged.RemoveDynamic(this, &UUI_SettingsMenu::HandleWindowModeChanged);
		WindowModeSetting->InitializeOptions(
			FText::FromString(TEXT("窗口模式")),
			{
				FText::FromString(TEXT("窗口化")),
				FText::FromString(TEXT("无边框")),
				FText::FromString(TEXT("全屏"))
			},
			GetWindowModeIndexFromSettings());
		WindowModeSetting->OnSelectionChanged.AddDynamic(this, &UUI_SettingsMenu::HandleWindowModeChanged);
	}

	if (ResolutionSetting)
	{
		ResolutionSetting->OnSelectionChanged.RemoveDynamic(this, &UUI_SettingsMenu::HandleResolutionChanged);
		ResolutionOptions = BuildResolutionOptions();
		ResolutionSetting->InitializeOptions(
			FText::FromString(TEXT("分辨率")),
			GetResolutionTextOptions(ResolutionOptions),
			GetResolutionIndexFromSettings());
		ResolutionSetting->OnSelectionChanged.AddDynamic(this, &UUI_SettingsMenu::HandleResolutionChanged);
	}

	if (ImageQualitySetting)
	{
		ImageQualitySetting->OnSelectionChanged.RemoveDynamic(this, &UUI_SettingsMenu::HandleImageQualityChanged);
		ImageQualitySetting->InitializeOptions(
			FText::FromString(TEXT("图像质量")),
			{
				FText::FromString(TEXT("低")),
				FText::FromString(TEXT("中")),
				FText::FromString(TEXT("高")),
				FText::FromString(TEXT("极致"))
			},
			GetImageQualityIndexFromSettings());
		ImageQualitySetting->OnSelectionChanged.AddDynamic(this, &UUI_SettingsMenu::HandleImageQualityChanged);
	}

	if (FrameRateSetting)
	{
		FrameRateSetting->OnSelectionChanged.RemoveDynamic(this, &UUI_SettingsMenu::HandleFrameRateChanged);
		FrameRateSetting->InitializeOptions(
			FText::FromString(TEXT("帧率")),
			{
				FText::FromString(TEXT("30")),
				FText::FromString(TEXT("60")),
				FText::FromString(TEXT("120")),
				FText::FromString(TEXT("144")),
				FText::FromString(TEXT("无限制"))
			},
			GetFrameRateIndexFromSettings());
		FrameRateSetting->OnSelectionChanged.AddDynamic(this, &UUI_SettingsMenu::HandleFrameRateChanged);
	}
	
	if (BGMSetting)
	{
		BGMSetting->OnSelectionChanged.RemoveDynamic(this, &UUI_SettingsMenu::HandleBGMChanged);
		BGMSetting->InitializeOptions(
			FText::FromString(TEXT("音乐")),
			GetVolumeTextOptions(),
			GetVolumeIndex(BGMVolume));
		BGMSetting->OnSelectionChanged.AddDynamic(this, &UUI_SettingsMenu::HandleBGMChanged);
	}
	
	if (SoundSetting)
	{
		SoundSetting->OnSelectionChanged.RemoveDynamic(this, &UUI_SettingsMenu::HandleSoundChanged);
		SoundSetting->InitializeOptions(
			FText::FromString(TEXT("音效")),
			GetVolumeTextOptions(),
			GetVolumeIndex(SFXVolume));
		SoundSetting->OnSelectionChanged.AddDynamic(this, &UUI_SettingsMenu::HandleSoundChanged);
	}

	if (BackBtn)
	{
		BackBtn->OnClicked.RemoveDynamic(this, &UUI_SettingsMenu::HandleBackClicked);
		BackBtn->OnClicked.AddDynamic(this, &UUI_SettingsMenu::HandleBackClicked);
	}
}

void UUI_SettingsMenu::HandleWindowModeChanged(int32 NewIndex, const FText& NewText)
{
	static_cast<void>(NewText);

	ApplyWindowModeByIndex(NewIndex);
}

void UUI_SettingsMenu::HandleResolutionChanged(int32 NewIndex, const FText& NewText)
{
	static_cast<void>(NewText);

	ApplyResolutionByIndex(NewIndex);
}

void UUI_SettingsMenu::HandleImageQualityChanged(int32 NewIndex, const FText& NewText)
{
	static_cast<void>(NewText);

	ApplyImageQualityByIndex(NewIndex);
}

void UUI_SettingsMenu::HandleFrameRateChanged(int32 NewIndex, const FText& NewText)
{
	static_cast<void>(NewText);

	ApplyFrameRateByIndex(NewIndex);
}

void UUI_SettingsMenu::HandleBGMChanged(int32 NewIndex, const FText& NewText)
{
	static_cast<void>(NewText);

	if (UBgmSubsystem* BgmSubsystem = UBgmSubsystem::Get(this))
	{
		BgmSubsystem->SetBGMVolume(GetVolumeByIndex(NewIndex));
	}

	SaveCurrentVolumes();
}

void UUI_SettingsMenu::HandleSoundChanged(int32 NewIndex, const FText& NewText)
{
	static_cast<void>(NewText);

	if (UBgmSubsystem* BgmSubsystem = UBgmSubsystem::Get(this))
	{
		BgmSubsystem->SetSFXVolume(GetVolumeByIndex(NewIndex));
	}

	SaveCurrentVolumes();
}

void UUI_SettingsMenu::HandleBackClicked()
{
	RemoveFromParent();
}

void UUI_SettingsMenu::SaveCurrentVolumes() const
{
	UGsPlayerSaveGame::SaveVolumes(GetBGMVolume(), GetSFXVolume());
}

TArray<FIntPoint> UUI_SettingsMenu::BuildResolutionOptions() const
{
	TArray<FIntPoint> Options = GetDefaultResolutionOptions();

	const UGameUserSettings* GameUserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	FIntPoint CurrentResolution = GameUserSettings ? GameUserSettings->GetScreenResolution() : FIntPoint::ZeroValue;
	if (!IsValidResolution(CurrentResolution))
	{
		CurrentResolution = GetValidWindowModeResolution(
			GameUserSettings,
			GameUserSettings ? GameUserSettings->GetFullscreenMode() : EWindowMode::Windowed);
	}

	if (IsValidResolution(CurrentResolution) && FindResolutionIndex(Options, CurrentResolution) == INDEX_NONE)
	{
		Options.Insert(CurrentResolution, 0);
	}

	return Options;
}

int32 UUI_SettingsMenu::GetWindowModeIndexFromSettings() const
{
	const UGameUserSettings* GameUserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!GameUserSettings)
	{
		return 0;
	}

	switch (GameUserSettings->GetFullscreenMode())
	{
	case EWindowMode::Windowed:
		return 0;
	case EWindowMode::WindowedFullscreen:
		return 1;
	case EWindowMode::Fullscreen:
		return 2;
	default:
		return 0;
	}
}

int32 UUI_SettingsMenu::GetResolutionIndexFromSettings() const
{
	const UGameUserSettings* GameUserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!GameUserSettings)
	{
		return 0;
	}

	FIntPoint CurrentResolution = GameUserSettings->GetScreenResolution();
	if (!IsValidResolution(CurrentResolution))
	{
		CurrentResolution = GetValidWindowModeResolution(GameUserSettings, GameUserSettings->GetFullscreenMode());
	}

	const int32 ResolutionIndex = FindResolutionIndex(ResolutionOptions, CurrentResolution);
	return ResolutionIndex == INDEX_NONE ? 0 : ResolutionIndex;
}

int32 UUI_SettingsMenu::GetImageQualityIndexFromSettings() const
{
	const UGameUserSettings* GameUserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!GameUserSettings)
	{
		return 3;
	}

	const int32 QualityIndex = GameUserSettings->GetOverallScalabilityLevel();
	return QualityIndex >= 0 ? FMath::Clamp(QualityIndex, 0, 3) : 3;
}

int32 UUI_SettingsMenu::GetFrameRateIndexFromSettings() const
{
	const UGameUserSettings* GameUserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!GameUserSettings)
	{
		return 4;
	}

	const float FrameRateLimit = GameUserSettings->GetFrameRateLimit();
	if (FrameRateLimit <= 0.f)
	{
		return 4;
	}

	int32 ClosestIndex = 0;
	float ClosestDistance = FMath::Abs(FrameRateLimit - GetFrameRateLimitByIndex(0));
	for (int32 Index = 1; Index < 4; ++Index)
	{
		const float Distance = FMath::Abs(FrameRateLimit - GetFrameRateLimitByIndex(Index));
		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestIndex = Index;
		}
	}

	return ClosestIndex;
}

void UUI_SettingsMenu::ApplyWindowModeByIndex(int32 NewIndex) const
{
	UGameUserSettings* GameUserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!GameUserSettings)
	{
		return;
	}

	EWindowMode::Type WindowMode = EWindowMode::Windowed;
	switch (NewIndex)
	{
	case 0:
		WindowMode = EWindowMode::Windowed;
		break;
	case 1:
		WindowMode = EWindowMode::WindowedFullscreen;
		break;
	case 2:
		WindowMode = EWindowMode::Fullscreen;
		break;
	default:
		break;
	}

	const FIntPoint TargetResolution = GetValidWindowModeResolution(GameUserSettings, WindowMode);

	GameUserSettings->SetFullscreenMode(WindowMode);
	GameUserSettings->SetScreenResolution(TargetResolution);
	UGameUserSettings::RequestResolutionChange(TargetResolution.X, TargetResolution.Y, WindowMode, false);
	GameUserSettings->ConfirmVideoMode();
	GameUserSettings->ApplySettings(false);
	GameUserSettings->SaveSettings();
}

void UUI_SettingsMenu::ApplyResolutionByIndex(int32 NewIndex) const
{
	UGameUserSettings* GameUserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!GameUserSettings || !ResolutionOptions.IsValidIndex(NewIndex))
	{
		return;
	}

	const FIntPoint TargetResolution = ResolutionOptions[NewIndex];
	GameUserSettings->SetScreenResolution(TargetResolution);
	UGameUserSettings::RequestResolutionChange(TargetResolution.X, TargetResolution.Y, GameUserSettings->GetFullscreenMode(), false);
	GameUserSettings->ConfirmVideoMode();
	GameUserSettings->ApplySettings(false);
	GameUserSettings->SaveSettings();
}

void UUI_SettingsMenu::ApplyImageQualityByIndex(int32 NewIndex) const
{
	UGameUserSettings* GameUserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!GameUserSettings)
	{
		return;
	}

	GameUserSettings->SetOverallScalabilityLevel(FMath::Clamp(NewIndex, 0, 3));
	GameUserSettings->ApplySettings(false);
	GameUserSettings->SaveSettings();
}

void UUI_SettingsMenu::ApplyFrameRateByIndex(int32 NewIndex) const
{
	UGameUserSettings* GameUserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!GameUserSettings)
	{
		return;
	}

	GameUserSettings->SetFrameRateLimit(GetFrameRateLimitByIndex(NewIndex));
	GameUserSettings->ApplySettings(false);
	GameUserSettings->SaveSettings();
}

float UUI_SettingsMenu::GetBGMVolume() const
{
	return BGMSetting ? GetVolumeByIndex(BGMSetting->GetCurrentIndex()) : 1.f;
}

float UUI_SettingsMenu::GetSFXVolume() const
{
	return SoundSetting ? GetVolumeByIndex(SoundSetting->GetCurrentIndex()) : 1.f;
}
