// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerUI.generated.h"

class UProgressBar;
class AGsPlayer;
class UGsPauseMenuUI;
class UGsRespawnHintUI;
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

	/** 暂停界面*/
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGsPauseMenuUI> PauseWidget;

	/** 复活后显示的提示 UI 蓝图类 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Player UI|Respawn Hint")
	TSubclassOf<UGsRespawnHintUI> RespawnHintWidgetClass;
	
	UPROPERTY()
	TObjectPtr<UGsPauseMenuUI> NewPauseWidget;

	UPROPERTY(Transient)
	TObjectPtr<UGsRespawnHintUI> RespawnHintWidget;

private:
	UPROPERTY(Transient)
	TObjectPtr<AGsPlayer> BoundPlayer;

	UFUNCTION()
	void HandlePlayerDeath();

	UFUNCTION()
	void HandlePlayerRespawn();

	UFUNCTION()
	void HandlePlayerRespawnHint(const FText& HintText);

	void SetDeathWidgetVisible(bool bVisible);
	void UpdateSkillCooldown();
	void ShowRespawnHint(const FText& HintText);
};
