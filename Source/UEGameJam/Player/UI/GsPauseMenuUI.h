// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GsPauseMenuUI.generated.h"

class UButton;

/**
 * 游戏内暂停菜单 UI。
 */
UCLASS()
class UEGAMEJAM_API UGsPauseMenuUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Player UI|Pause")
	void ShowPauseMenu();

	UFUNCTION(BlueprintCallable, Category="Player UI|Pause")
	void HidePauseMenu();

	UFUNCTION(BlueprintCallable, Category="Player UI|Pause")
	void TogglePauseMenu();

	UFUNCTION(BlueprintPure, Category="Player UI|Pause")
	bool IsPauseMenuVisible() const { return bIsPauseMenuVisible; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** 继续游戏按钮，需要在 Widget 蓝图中命名为 ResumeButton */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ResumeButton;

	/** 返回主界面按钮，需要在 Widget 蓝图中命名为 ReturnMainMenuButton */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReturnMainMenuButton;

private:
	bool bIsPauseMenuVisible = false;

	UFUNCTION()
	void HandleResumeClicked();

	UFUNCTION()
	void HandleReturnMainMenuClicked();
};
