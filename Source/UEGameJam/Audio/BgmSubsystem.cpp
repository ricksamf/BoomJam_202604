#include "Audio/BgmSubsystem.h"

#include "Audio/AudioDataAsset.h"
#include "Components/AudioComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Game/GsPlayerSaveGame.h"
#include "Settings/GsProjectResourceSettings.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

void UBgmSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (UGsPlayerSaveGame* SaveGame = UGsPlayerSaveGame::LoadOrCreate())
	{
		BGMVolume = SaveGame->GetBGMVolume();
		SFXVolume = SaveGame->GetSFXVolume();
	}

	ApplySoundClassVolumes();
}

void UBgmSubsystem::Deinitialize()
{
	StopBGM();
	RestoreOriginalSoundClassVolumes();
	Super::Deinitialize();
}

UBgmSubsystem* UBgmSubsystem::Get(const UObject* WorldContext)
{
	if (!WorldContext)
	{
		return nullptr;
	}

	UWorld* World = WorldContext->GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UBgmSubsystem>() : nullptr;
}

void UBgmSubsystem::PlayDefaultBGM()
{
	ClearIntroTimer();
	bCombatLoopStarted = false;
	StopComponent(IntroBGMComponent, 0.f);
	StopComponent(CombatBGMComponent, 0.f);
	StopComponent(SurfaceBGMComponent, 0.f);
	StopComponent(RealmBGMComponent, 0.f);

	UAudioDataAsset* AudioData = GetAudioData();
	if (!AudioData || !AudioData->DefaultBGM)
	{
		StopComponent(DefaultBGMComponent, StopFadeOutTime);
		return;
	}

	StopComponent(DefaultBGMComponent, 0.f);
	DefaultBGMComponent = CreateBGMComponent(AudioData->DefaultBGM);
	if (DefaultBGMComponent)
	{
		DefaultBGMComponent->FadeIn(DefaultFadeInTime, 1.f);
	}
}

void UBgmSubsystem::PlayCombatBGM(ERealmType InitialRealm)
{
	ClearIntroTimer();
	PendingCombatRealm = InitialRealm;
	CurrentCombatRealm = InitialRealm;
	bCombatLoopStarted = false;

	StopComponent(DefaultBGMComponent, StopFadeOutTime);
	StopComponent(IntroBGMComponent, 0.f);
	StopComponent(CombatBGMComponent, 0.f);
	StopComponent(SurfaceBGMComponent, 0.f);
	StopComponent(RealmBGMComponent, 0.f);

	UAudioDataAsset* AudioData = GetAudioData();
	if (!AudioData)
	{
		return;
	}

	if (!AudioData->IntroBGM)
	{
		StartCombatLoop();
		return;
	}

	IntroBGMComponent = CreateBGMComponent(AudioData->IntroBGM);
	if (!IntroBGMComponent)
	{
		StartCombatLoop();
		return;
	}

	IntroBGMComponent->Play();
	IntroBGMComponent->SetVolumeMultiplier(1.f);

	const float IntroDuration = AudioData->IntroBGM->GetDuration();
	if (IntroDuration > 0.f)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(IntroTimerHandle, this, &UBgmSubsystem::StartCombatLoop, IntroDuration, false);
		}
		else
		{
			StartCombatLoop();
		}
	}
	else
	{
		StartCombatLoop();
	}
}

void UBgmSubsystem::SwitchCombatBGM(ERealmType NewRealm)
{
	PendingCombatRealm = NewRealm;

	if (!bCombatLoopStarted)
	{
		return;
	}

	CurrentCombatRealm = NewRealm;
	ApplyCombatMix(CombatCrossFadeTime);
}

void UBgmSubsystem::StopBGM()
{
	ClearIntroTimer();
	bCombatLoopStarted = false;

	StopComponent(DefaultBGMComponent, StopFadeOutTime);
	StopComponent(IntroBGMComponent, StopFadeOutTime);
	StopComponent(CombatBGMComponent, StopFadeOutTime);
	StopComponent(SurfaceBGMComponent, StopFadeOutTime);
	StopComponent(RealmBGMComponent, StopFadeOutTime);
}

void UBgmSubsystem::SetBGMVolume(float NewVolume)
{
	BGMVolume = FMath::Clamp(NewVolume, 0.f, 1.f);
	ApplyBGMVolume();
}

void UBgmSubsystem::SetSFXVolume(float NewVolume)
{
	SFXVolume = FMath::Clamp(NewVolume, 0.f, 1.f);
	ApplySoundClassVolumes();
}

UAudioDataAsset* UBgmSubsystem::GetAudioData() const
{
	const UGsProjectResourceSettings* ResourceSettings = GetDefault<UGsProjectResourceSettings>();
	return ResourceSettings ? ResourceSettings->AudioDataAsset.LoadSynchronous() : nullptr;
}

UAudioComponent* UBgmSubsystem::CreateBGMComponent(USoundBase* Sound)
{
	if (!Sound)
	{
		return nullptr;
	}

	Sound->VirtualizationMode = EVirtualizationMode::PlayWhenSilent;

	UAudioComponent* AudioComponent = UGameplayStatics::CreateSound2D(this, Sound, 1.f, 1.f, 0.f, nullptr, false, false);
	if (AudioComponent)
	{
		AudioComponent->bIsUISound = true;
		AudioComponent->SetVolumeMultiplier(1.f);
	}
	return AudioComponent;
}

void UBgmSubsystem::StartCombatLoop()
{
	ClearIntroTimer();
	StopComponent(IntroBGMComponent, 0.f);

	UAudioDataAsset* AudioData = GetAudioData();
	if (!AudioData)
	{
		return;
	}

	CurrentCombatRealm = PendingCombatRealm;
	bCombatLoopStarted = true;

	CombatBGMComponent = CreateBGMComponent(AudioData->CombatBGM);
	SurfaceBGMComponent = CreateBGMComponent(AudioData->SurfaceBGM);
	RealmBGMComponent = CreateBGMComponent(AudioData->RealmBGM);

	if (CombatBGMComponent)
	{
		CombatBGMComponent->Play();
	}
	if (SurfaceBGMComponent)
	{
		SurfaceBGMComponent->Play();
	}
	if (RealmBGMComponent)
	{
		RealmBGMComponent->Play();
	}

	ApplyCombatMix(0.f);
}

void UBgmSubsystem::ApplyCombatMix(float FadeTime)
{
	const bool bPlaySurface = CurrentCombatRealm == ERealmType::Surface;

	if (SurfaceBGMComponent)
	{
		SurfaceBGMComponent->AdjustVolume(FadeTime, bPlaySurface ? 1.f : MutedCombatLayerVolume);
	}
	if (RealmBGMComponent)
	{
		RealmBGMComponent->AdjustVolume(FadeTime, bPlaySurface ? MutedCombatLayerVolume : 1.f);
	}
}

void UBgmSubsystem::ApplyBGMVolume()
{
	ApplySoundClassVolumes();
}

void UBgmSubsystem::ApplySoundClassVolumes()
{
	CacheSoundClasses();

	if (CachedBGMSoundClass)
	{
		CachedBGMSoundClass->Properties.Volume = OriginalBGMClassVolume * BGMVolume;
	}

	if (CachedSFXSoundClass)
	{
		CachedSFXSoundClass->Properties.Volume = OriginalSFXClassVolume * SFXVolume;
	}
}

void UBgmSubsystem::CacheSoundClasses()
{
	const UGsProjectResourceSettings* ResourceSettings = GetDefault<UGsProjectResourceSettings>();
	if (!ResourceSettings)
	{
		return;
	}

	if (!CachedBGMSoundClass)
	{
		CachedBGMSoundClass = ResourceSettings->BGMSoundClass.LoadSynchronous();
	}

	if (!CachedSFXSoundClass)
	{
		CachedSFXSoundClass = ResourceSettings->SFXSoundClass.LoadSynchronous();
	}

	if (CachedBGMSoundClass && !bCachedOriginalBGMVolume)
	{
		OriginalBGMClassVolume = CachedBGMSoundClass->Properties.Volume;
		bCachedOriginalBGMVolume = true;
	}

	if (CachedSFXSoundClass && !bCachedOriginalSFXVolume)
	{
		OriginalSFXClassVolume = CachedSFXSoundClass->Properties.Volume;
		bCachedOriginalSFXVolume = true;
	}
}

void UBgmSubsystem::RestoreOriginalSoundClassVolumes()
{
	if (CachedBGMSoundClass && bCachedOriginalBGMVolume)
	{
		CachedBGMSoundClass->Properties.Volume = OriginalBGMClassVolume;
	}

	if (CachedSFXSoundClass && bCachedOriginalSFXVolume)
	{
		CachedSFXSoundClass->Properties.Volume = OriginalSFXClassVolume;
	}
}

void UBgmSubsystem::StopComponent(TObjectPtr<UAudioComponent>& Component, float FadeTime)
{
	if (!IsValid(Component))
	{
		Component = nullptr;
		return;
	}

	if (FadeTime > 0.f)
	{
		Component->FadeOut(FadeTime, 0.f);
	}
	else
	{
		Component->Stop();
	}

	Component = nullptr;
}

void UBgmSubsystem::ClearIntroTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(IntroTimerHandle);
	}
}
