// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI_MainMenu.generated.h"

class UButton;
class UImage;
class UMediaPlayer;
class UMediaSource;
class UMediaTexture;
class UTextBlock;
class UWidget;
class UUI_Login;
class UUI_SettingsMenu;

/**
 * 主界面
 */
UCLASS()
class UEGAMEJAM_API UUI_MainMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UUI_MainMenu(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** 开始游戏按钮 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> StartBtn;

	/** 设置按钮 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> SetBtn;

	/** 退出游戏按钮 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> ExitBtn;

	/** 点击开始游戏后进入的关卡名 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Main Menu")
	FName StartLevelName = TEXT("LEVEL_01");

	/** 设置界面蓝图类，点击设置按钮时创建并显示 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Main Menu")
	TSubclassOf<UUI_SettingsMenu> SettingsWidgetClass;

	/** 登录界面蓝图类，非调试模式点击开始游戏时创建并显示 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Main Menu")
	TSubclassOf<UUI_Login> LoginWidgetClass;

	/** 当前创建的设置界面实例 */
	UPROPERTY(Transient)
	TObjectPtr<UUI_SettingsMenu> SettingsWidget;

	/** 当前创建的登录界面实例 */
	UPROPERTY(Transient)
	TObjectPtr<UUI_Login> LoginWidget;

	/** 主菜单停留多久后自动播放吸引模式视频，单位秒 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Main Menu|Attract", meta=(ClampMin="0.0"))
	float IdleVideoDelay = 60.f;

	/** 吸引模式循环播放的视频源 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Main Menu|Attract")
	TObjectPtr<UMediaSource> AttractVideoSource;

	/** 吸引模式使用的媒体播放器；为空时会运行时创建一个 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Main Menu|Attract")
	TObjectPtr<UMediaPlayer> AttractMediaPlayer;

	/** 吸引模式视频层根控件，可选绑定；未配置时会自动显示视频控件的父层 */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UWidget> AttractVideoRoot;

	/** 吸引模式视频显示控件，可选绑定；会被设置为运行时视频纹理 */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UImage> AttractVideoImage;

	/** 吸引模式点击返回按钮，可选绑定；应覆盖整个屏幕 */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> AttractInputCatcher;

	/** 吸引模式点击提示文本，可选绑定；播放视频时会以白色闪烁 */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> AttractInputCatcherText;

private:
	UFUNCTION()
	void HandleStartClicked();

	UFUNCTION()
	void HandleSettingsClicked();

	UFUNCTION()
	void HandleExitClicked();

	UFUNCTION()
	void HandleAttractClicked();

	UFUNCTION()
	void HandleSettingsClosed();

	UFUNCTION()
	void HandleLoginClosed();

	UFUNCTION()
	void HandleAttractMediaOpened(FString OpenedUrl);

	UFUNCTION()
	void HandleAttractMediaOpenFailed(FString FailedUrl);

	void StartIdleTimer();
	void StopIdleTimer();
	void EnterAttractMode();
	void ExitAttractMode();
	void StopAttractVideo();
	void SetAttractLayerVisibility(bool bVisible);
	void SetMenuButtonsEnabled(bool bEnabled);
	void ShowAttractVideoParents();
	void RestoreAttractVideoParents();
	bool CanPlayAttractVideo() const;
	UMediaPlayer* GetAttractMediaPlayer();
	void PrepareAttractVideoImage();
	void BindSettingsClosed();
	void BindLoginClosed();
	void ShowLoginWidget();
	void OpenStartLevel() const;
	bool IsDebugStartEnabled() const;

	UPROPERTY(Transient)
	TObjectPtr<UMediaPlayer> RuntimeAttractMediaPlayer;

	UPROPERTY(Transient)
	TObjectPtr<UMediaTexture> RuntimeAttractMediaTexture;

	FTimerHandle IdleVideoTimerHandle;
	TMap<TWeakObjectPtr<UWidget>, ESlateVisibility> SavedAttractParentVisibilities;
	float AttractTextBlinkDuration = 1.5f;
	float AttractTextBlinkElapsed = 0.f;
	bool bIsAttractModeActive = false;
};
