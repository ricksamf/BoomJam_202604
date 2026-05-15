// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerUI.generated.h"

class UProgressBar;
class AGsPlayer;
class UGsPauseMenuUI;
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

	UFUNCTION(BlueprintCallable, Category="Player UI|Pause")
	void ShowPauseMenu();

	UFUNCTION(BlueprintCallable, Category="Player UI|Pause")
	void HidePauseMenu();

	UFUNCTION(BlueprintCallable, Category="Player UI|Pause")
	void TogglePauseMenu();

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;

	/** 玩家死亡时显示的死亡界面，需要在 Widget 蓝图中命名为 DeathWidget */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> DeathWidget;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> SkillCd;

	/** 暂停界面根节点，需要在 Widget 蓝图中命名为 PauseWidget */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UGsPauseMenuUI> PauseWidget;

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
