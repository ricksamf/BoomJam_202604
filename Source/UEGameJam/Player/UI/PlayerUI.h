// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerUI.generated.h"

class UProgressBar;
class AGsPlayer;
class UWidget;

/**
 *  纯玩家侧角色 UI 基类
 */
UCLASS(Abstract)
class UEGAMEJAM_API UPlayerUI : public UUserWidget
{
	GENERATED_BODY()

public:
	void BindPlayer(AGsPlayer* InPlayer);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;

	/** 玩家死亡时显示的死亡界面，需要在 Widget 蓝图中命名为 DeathWidget */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> DeathWidget;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> SkillCd;

private:
	UPROPERTY(Transient)
	TObjectPtr<AGsPlayer> BoundPlayer;

	UFUNCTION()
	void HandlePlayerDeath();

	UFUNCTION()
	void HandlePlayerRespawn();

	void SetDeathWidgetVisible(bool bVisible);
	void UpdateSkillCooldown();
};
