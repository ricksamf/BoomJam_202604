// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RealmTagComponent.h"
#include "BgmSubsystem.generated.h"

class UAudioComponent;
class UAudioDataAsset;
class USoundBase;
class USoundClass;

/**
 * 全局BGM子系统
 */
UCLASS()
class UEGAMEJAM_API UBgmSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 便捷访问器 */
	UFUNCTION(BlueprintPure, Category="Audio|BGM", meta=(WorldContext="WorldContext"))
	static UBgmSubsystem* Get(const UObject* WorldContext);

	/** 播放默认BGM，用于菜单或进游戏后的普通状态 */
	UFUNCTION(BlueprintCallable, Category="Audio|BGM")
	void PlayDefaultBGM();

	/** 播放战斗BGM，先播放Intro，再进入指定世界的循环BGM */
	UFUNCTION(BlueprintCallable, Category="Audio|BGM")
	void PlayCombatBGM(ERealmType InitialRealm);

	/** 切换战斗BGM所属世界，只做淡入淡出并保持播放进度 */
	UFUNCTION(BlueprintCallable, Category="Audio|BGM")
	void SwitchCombatBGM(ERealmType NewRealm);

	/** 停止所有BGM */
	UFUNCTION(BlueprintCallable, Category="Audio|BGM")
	void StopBGM();

	/** 设置BGM音量，范围0到1 */
	UFUNCTION(BlueprintCallable, Category="Audio|Volume")
	void SetBGMVolume(float NewVolume);

	/** 获取当前BGM音量 */
	UFUNCTION(BlueprintPure, Category="Audio|Volume")
	float GetBGMVolume() const { return BGMVolume; }

	/** 设置音效音量，范围0到1 */
	UFUNCTION(BlueprintCallable, Category="Audio|Volume")
	void SetSFXVolume(float NewVolume);

	/** 获取当前音效音量 */
	UFUNCTION(BlueprintPure, Category="Audio|Volume")
	float GetSFXVolume() const { return SFXVolume; }

protected:
	/** 默认BGM淡入时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio|BGM", meta=(ClampMin="0.0"))
	float DefaultFadeInTime = 1.f;

	/** BGM停止时的淡出时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio|BGM", meta=(ClampMin="0.0"))
	float StopFadeOutTime = 1.f;

	/** 表世界和里世界战斗BGM切换时的淡入淡出时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio|BGM", meta=(ClampMin="0.0"))
	float CombatCrossFadeTime = 1.f;

private:
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> DefaultBGMComponent;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> IntroBGMComponent;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> CombatBGMComponent;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> SurfaceBGMComponent;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> RealmBGMComponent;

	UPROPERTY(Transient)
	TObjectPtr<USoundClass> CachedBGMSoundClass;

	UPROPERTY(Transient)
	TObjectPtr<USoundClass> CachedSFXSoundClass;

	FTimerHandle IntroTimerHandle;

	ERealmType PendingCombatRealm = ERealmType::Surface;
	ERealmType CurrentCombatRealm = ERealmType::Surface;
	bool bCombatLoopStarted = false;
	bool bCachedOriginalBGMVolume = false;
	bool bCachedOriginalSFXVolume = false;
	float BGMVolume = 1.f;
	float SFXVolume = 1.f;
	float OriginalBGMClassVolume = 1.f;
	float OriginalSFXClassVolume = 1.f;
	float MutedCombatLayerVolume = 0.01f;

	UAudioDataAsset* GetAudioData() const;
	UAudioComponent* CreateBGMComponent(USoundBase* Sound);
	void StartCombatLoop();
	void ApplyCombatMix(float FadeTime);
	void ApplyBGMVolume();
	void ApplySoundClassVolumes();
	void CacheSoundClasses();
	void RestoreOriginalSoundClassVolumes();
	void StopComponent(TObjectPtr<UAudioComponent>& Component, float FadeTime);
	void ClearIntroTimer();
};
