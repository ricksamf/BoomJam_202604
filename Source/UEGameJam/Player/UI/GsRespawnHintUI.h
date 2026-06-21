// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GsRespawnHintUI.generated.h"

class UCanvasPanel;
class UTextBlock;

enum class EGsRespawnHintAnimationState : uint8
{
	None,
	SlidingIn,
	Staying,
	SlidingOut
};

/**
 * 玩家复活提示 UI 基类。
 */
UCLASS(Abstract, Blueprintable)
class UEGAMEJAM_API UGsRespawnHintUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Player UI|Respawn Hint")
	void SetHintText(const FText& InHintText);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** 提示文本控件，需要在 Widget 蓝图中命名为 HintText */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HintText;

	/** 提示移动的画布控件，需要在 Widget 蓝图中命名为 HintCanvasPanel */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> HintCanvasPanel;

	/** 提示进入和退出时横向移动的像素距离 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Player UI|Respawn Hint", meta = (ClampMin = 0.0))
	float SlideDistance = 568.0f;

	/** 提示进入动画持续时间 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Player UI|Respawn Hint", meta = (ClampMin = 0.0))
	float SlideInDuration = 0.2f;

	/** 提示完全显示后的停留时间 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Player UI|Respawn Hint", meta = (ClampMin = 0.0))
	float StayDuration = 1.0f;

	/** 提示退出动画持续时间 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Player UI|Respawn Hint", meta = (ClampMin = 0.0))
	float SlideOutDuration = 0.2f;

	/** 设置复活提示文本后触发，蓝图可在这里播放提示动画 */
	UFUNCTION(BlueprintImplementableEvent, Category="Player UI|Respawn Hint", meta = (DisplayName = "On Hint Text Set"))
	void BP_OnHintTextSet(const FText& InHintText);

private:
	FText CurrentHintText;
	EGsRespawnHintAnimationState AnimationState = EGsRespawnHintAnimationState::None;
	float AnimationElapsedTime = 0.0f;

	void ApplyHintText();
	void StartHintAnimation();
	void StopHintAnimation();
	void SetHintCanvasOffset(float OffsetX);
};
