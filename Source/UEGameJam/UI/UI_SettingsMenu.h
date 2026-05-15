// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI_SettingsMenu.generated.h"

class UButton;
class UUI_SettingsWidget;

/**
 * 设置界面
 */
UCLASS()
class UEGAMEJAM_API UUI_SettingsMenu : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	/** 窗口模式设置条，用于切换窗口化、无边框和全屏 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UUI_SettingsWidget> WindowModeSetting;

	/** 分辨率设置条，用于切换常用屏幕分辨率 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UUI_SettingsWidget> ResolutionSetting;

	/** 图像质量设置条，用于切换整体画质预设 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UUI_SettingsWidget> ImageQualitySetting;

	/** 帧率设置条，用于限制游戏最高帧率 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UUI_SettingsWidget> FrameRateSetting;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UUI_SettingsWidget> BGMSetting;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UUI_SettingsWidget> SoundSetting;

	/** 返回主菜单按钮 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> BackBtn;

private:
	UFUNCTION()
	void HandleWindowModeChanged(int32 NewIndex, const FText& NewText);

	UFUNCTION()
	void HandleResolutionChanged(int32 NewIndex, const FText& NewText);

	UFUNCTION()
	void HandleImageQualityChanged(int32 NewIndex, const FText& NewText);

	UFUNCTION()
	void HandleFrameRateChanged(int32 NewIndex, const FText& NewText);
	
	UFUNCTION()
	void HandleBGMChanged(int32 NewIndex, const FText& NewText);
	
	UFUNCTION()
	void HandleSoundChanged(int32 NewIndex, const FText& NewText);

	UFUNCTION()
	void HandleBackClicked();

	void SaveCurrentVolumes() const;
	TArray<FIntPoint> BuildResolutionOptions() const;
	int32 GetWindowModeIndexFromSettings() const;
	int32 GetResolutionIndexFromSettings() const;
	int32 GetImageQualityIndexFromSettings() const;
	int32 GetFrameRateIndexFromSettings() const;
	void ApplyWindowModeByIndex(int32 NewIndex) const;
	void ApplyResolutionByIndex(int32 NewIndex) const;
	void ApplyImageQualityByIndex(int32 NewIndex) const;
	void ApplyFrameRateByIndex(int32 NewIndex) const;
	float GetBGMVolume() const;
	float GetSFXVolume() const;

	UPROPERTY(Transient)
	TArray<FIntPoint> ResolutionOptions;
};
