// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GsProjectResourceSettings.generated.h"

class UAudioDataAsset;
class UDataTable;
class USoundClass;

/**
 * 项目资源配置。
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="资源配置"))
class UEGAMEJAM_API UGsProjectResourceSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override;

	/** 全局音频资源配置，用于集中指定项目使用的音效资源DataAsset */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Audio")
	TSoftObjectPtr<UAudioDataAsset> AudioDataAsset;

	/** BGM声音分类，用于设置菜单统一调节背景音乐音量 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Audio")
	TSoftObjectPtr<USoundClass> BGMSoundClass;

	/** 音效声音分类，用于设置菜单统一调节战斗和操作音效音量 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Audio")
	TSoftObjectPtr<USoundClass> SFXSoundClass;

	/** 登录界面词库表，用于配置随机名字和违禁词 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Login")
	TSoftObjectPtr<UDataTable> LoginWordTable;

	/** 调试模式下跳过登录界面，主菜单点击开始游戏后直接进入关卡 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Debug")
	bool bDebugSkipLogin = false;

	/** 排行榜关卡全局限时时长，单位秒 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Rank", meta=(ClampMin=1.0, Units="s"))
	float RankTimeLimitSeconds = 180.0f;
	
	/** 是否显示敌人相关的屏幕调试信息 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Debug")
	bool bShowEnemy = true;
};
