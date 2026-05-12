// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI_SettingsMenu.generated.h"

class UButton;
class USlider;

/**
 * 设置界面
 */
UCLASS()
class UEGAMEJAM_API UUI_SettingsMenu : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	/** BGM音量滑动条，用于调节背景音乐大小 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<USlider> BGMSlider;

	/** 音效音量滑动条，用于调节战斗和操作音效大小 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<USlider> SFXSlider;

	/** 返回主菜单按钮 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> BackBtn;

private:
	UFUNCTION()
	void HandleBGMVolumeChanged(float Value);

	UFUNCTION()
	void HandleSFXVolumeChanged(float Value);

	UFUNCTION()
	void HandleBackClicked();

	void SaveCurrentVolumes() const;
};
