// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/UI/GsRespawnHintUI.h"

#include "Components/CanvasPanel.h"
#include "Components/TextBlock.h"

void UGsRespawnHintUI::NativeConstruct()
{
	Super::NativeConstruct();

	ApplyHintText();
	StopHintAnimation();
}

void UGsRespawnHintUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (AnimationState == EGsRespawnHintAnimationState::None)
	{
		return;
	}

	AnimationElapsedTime += InDeltaTime;

	if (AnimationState == EGsRespawnHintAnimationState::SlidingIn)
	{
		const float Alpha = SlideInDuration > 0.0f ? FMath::Clamp(AnimationElapsedTime / SlideInDuration, 0.0f, 1.0f) : 1.0f;
		SetHintCanvasOffset(FMath::Lerp(0.0f, -SlideDistance, Alpha));

		if (Alpha >= 1.0f)
		{
			AnimationState = EGsRespawnHintAnimationState::Staying;
			AnimationElapsedTime = 0.0f;
			SetHintCanvasOffset(-SlideDistance);
		}
		return;
	}

	if (AnimationState == EGsRespawnHintAnimationState::Staying)
	{
		SetHintCanvasOffset(-SlideDistance);
		if (AnimationElapsedTime >= StayDuration)
		{
			AnimationState = EGsRespawnHintAnimationState::SlidingOut;
			AnimationElapsedTime = 0.0f;
		}
		return;
	}

	if (AnimationState == EGsRespawnHintAnimationState::SlidingOut)
	{
		const float Alpha = SlideOutDuration > 0.0f ? FMath::Clamp(AnimationElapsedTime / SlideOutDuration, 0.0f, 1.0f) : 1.0f;
		SetHintCanvasOffset(FMath::Lerp(-SlideDistance, 0.0f, Alpha));

		if (Alpha >= 1.0f)
		{
			StopHintAnimation();
		}
	}
}

void UGsRespawnHintUI::SetHintText(const FText& InHintText)
{
	CurrentHintText = InHintText;
	ApplyHintText();
	StartHintAnimation();
	BP_OnHintTextSet(CurrentHintText);
}

void UGsRespawnHintUI::ApplyHintText()
{
	if (HintText)
	{
		HintText->SetText(CurrentHintText);
	}
}

void UGsRespawnHintUI::StartHintAnimation()
{
	AnimationState = EGsRespawnHintAnimationState::SlidingIn;
	AnimationElapsedTime = 0.0f;
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	SetHintCanvasOffset(0.0f);
}

void UGsRespawnHintUI::StopHintAnimation()
{
	AnimationState = EGsRespawnHintAnimationState::None;
	AnimationElapsedTime = 0.0f;
	SetHintCanvasOffset(0.0f);
	SetVisibility(ESlateVisibility::Hidden);
}

void UGsRespawnHintUI::SetHintCanvasOffset(float OffsetX)
{
	if (HintCanvasPanel)
	{
		HintCanvasPanel->SetRenderTranslation(FVector2D(OffsetX, 0.0f));
	}
}
