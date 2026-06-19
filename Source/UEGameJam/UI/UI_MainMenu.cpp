// Fill out your copyright notice in the Description page of Project Settings.


#include "UI_MainMenu.h"

#include "Audio/BgmSubsystem.h"
#include "UI_SettingsMenu.h"
#include "Components/Button.h"
#include "Components/ContentWidget.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MediaPlayer.h"
#include "MediaSource.h"
#include "MediaTexture.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

UUI_MainMenu::UUI_MainMenu(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);

	static ConstructorHelpers::FObjectFinder<UMediaSource> DefaultAttractVideoSource(
		TEXT("/Game/Movies/Unreal_Engine_Logo_Video_Animation.Unreal_Engine_Logo_Video_Animation"));
	if (DefaultAttractVideoSource.Succeeded())
	{
		AttractVideoSource = DefaultAttractVideoSource.Object;
	}
}

void UUI_MainMenu::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartBtn)
	{
		StartBtn->OnClicked.RemoveDynamic(this, &UUI_MainMenu::HandleStartClicked);
		StartBtn->OnClicked.AddDynamic(this, &UUI_MainMenu::HandleStartClicked);
	}

	if (SetBtn)
	{
		SetBtn->OnClicked.RemoveDynamic(this, &UUI_MainMenu::HandleSettingsClicked);
		SetBtn->OnClicked.AddDynamic(this, &UUI_MainMenu::HandleSettingsClicked);
	}

	if (ExitBtn)
	{
		ExitBtn->OnClicked.RemoveDynamic(this, &UUI_MainMenu::HandleExitClicked);
		ExitBtn->OnClicked.AddDynamic(this, &UUI_MainMenu::HandleExitClicked);
	}

	if (AttractInputCatcher)
	{
		AttractInputCatcher->OnClicked.RemoveDynamic(this, &UUI_MainMenu::HandleAttractClicked);
		AttractInputCatcher->OnClicked.AddDynamic(this, &UUI_MainMenu::HandleAttractClicked);
	}

	SetAttractLayerVisibility(false);
	SetMenuButtonsEnabled(true);
	EnterAttractMode();
}

void UUI_MainMenu::NativeDestruct()
{
	StopIdleTimer();
	StopAttractVideo();
	Super::NativeDestruct();
}

void UUI_MainMenu::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bIsAttractModeActive || !AttractInputCatcherText || AttractTextBlinkDuration <= 0.f)
	{
		return;
	}

	AttractTextBlinkElapsed = FMath::Fmod(AttractTextBlinkElapsed + InDeltaTime, AttractTextBlinkDuration);
	const float BlinkAlpha = 0.5f + 0.5f * FMath::Cos(AttractTextBlinkElapsed / AttractTextBlinkDuration * 2.f * PI);
	AttractInputCatcherText->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 1.f, 1.f, BlinkAlpha)));
}

FReply UUI_MainMenu::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsAttractModeActive)
	{
		ExitAttractMode();
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UUI_MainMenu::HandleStartClicked()
{
	StopIdleTimer();

	if (!StartLevelName.IsNone())
	{
		UGameplayStatics::OpenLevel(this, StartLevelName);
	}
}

void UUI_MainMenu::HandleSettingsClicked()
{
	StopIdleTimer();

	if (SettingsWidget)
	{
		BindSettingsClosed();
		SettingsWidget->SetVisibility(ESlateVisibility::Visible);

		if (!SettingsWidget->IsInViewport())
		{
			SettingsWidget->AddToViewport(10);
		}

		return;
	}

	if (!SettingsWidgetClass)
	{
		StartIdleTimer();
		return;
	}

	if (APlayerController* OwningPlayer = GetOwningPlayer())
	{
		SettingsWidget = CreateWidget<UUI_SettingsMenu>(OwningPlayer, SettingsWidgetClass);
	}
	else if (UWorld* World = GetWorld())
	{
		SettingsWidget = CreateWidget<UUI_SettingsMenu>(World, SettingsWidgetClass);
	}

	if (SettingsWidget)
	{
		BindSettingsClosed();
		SettingsWidget->AddToViewport(10);
	}
	else
	{
		StartIdleTimer();
	}
}

void UUI_MainMenu::HandleExitClicked()
{
	StopIdleTimer();
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UUI_MainMenu::HandleAttractClicked()
{
	if (bIsAttractModeActive)
	{
		ExitAttractMode();
	}
}

void UUI_MainMenu::HandleSettingsClosed()
{
	if (!IsInViewport() || bIsAttractModeActive)
	{
		return;
	}

	StartIdleTimer();
}

void UUI_MainMenu::HandleAttractMediaOpened(FString OpenedUrl)
{
	static_cast<void>(OpenedUrl);

	if (bIsAttractModeActive)
	{
		if (UMediaPlayer* MediaPlayer = GetAttractMediaPlayer())
		{
			MediaPlayer->Play();
		}
	}
}

void UUI_MainMenu::HandleAttractMediaOpenFailed(FString FailedUrl)
{
	static_cast<void>(FailedUrl);

	if (bIsAttractModeActive)
	{
		ExitAttractMode();
	}
}

void UUI_MainMenu::StartIdleTimer()
{
	StopIdleTimer();

	if (bIsAttractModeActive || IdleVideoDelay <= 0.f || !CanPlayAttractVideo())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(IdleVideoTimerHandle, this, &UUI_MainMenu::EnterAttractMode, IdleVideoDelay, false);
	}
}

void UUI_MainMenu::StopIdleTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(IdleVideoTimerHandle);
	}
}

void UUI_MainMenu::EnterAttractMode()
{
	if (bIsAttractModeActive || !CanPlayAttractVideo())
	{
		return;
	}

	StopIdleTimer();
	bIsAttractModeActive = true;
	SetMenuButtonsEnabled(false);
	SetAttractLayerVisibility(true);

	if (UBgmSubsystem* BgmSubsystem = UBgmSubsystem::Get(this))
	{
		BgmSubsystem->StopBGM();
	}

	UMediaPlayer* MediaPlayer = GetAttractMediaPlayer();
	if (!MediaPlayer)
	{
		ExitAttractMode();
		return;
	}

	PrepareAttractVideoImage();
	MediaPlayer->OnMediaOpened.RemoveDynamic(this, &UUI_MainMenu::HandleAttractMediaOpened);
	MediaPlayer->OnMediaOpened.AddDynamic(this, &UUI_MainMenu::HandleAttractMediaOpened);
	MediaPlayer->OnMediaOpenFailed.RemoveDynamic(this, &UUI_MainMenu::HandleAttractMediaOpenFailed);
	MediaPlayer->OnMediaOpenFailed.AddDynamic(this, &UUI_MainMenu::HandleAttractMediaOpenFailed);
	MediaPlayer->SetLooping(true);
	MediaPlayer->PlayOnOpen = true;

	if (!MediaPlayer->OpenSource(AttractVideoSource))
	{
		ExitAttractMode();
	}
}

void UUI_MainMenu::ExitAttractMode()
{
	if (!bIsAttractModeActive)
	{
		return;
	}

	bIsAttractModeActive = false;
	StopAttractVideo();
	SetAttractLayerVisibility(false);
	SetMenuButtonsEnabled(true);

	if (UBgmSubsystem* BgmSubsystem = UBgmSubsystem::Get(this))
	{
		BgmSubsystem->PlayDefaultBGM();
	}

	StartIdleTimer();
}

void UUI_MainMenu::StopAttractVideo()
{
	UMediaPlayer* MediaPlayer = AttractMediaPlayer ? AttractMediaPlayer.Get() : RuntimeAttractMediaPlayer.Get();
	if (MediaPlayer)
	{
		MediaPlayer->OnMediaOpened.RemoveDynamic(this, &UUI_MainMenu::HandleAttractMediaOpened);
		MediaPlayer->OnMediaOpenFailed.RemoveDynamic(this, &UUI_MainMenu::HandleAttractMediaOpenFailed);
		MediaPlayer->Close();
	}
}

void UUI_MainMenu::SetAttractLayerVisibility(bool bVisible)
{
	const ESlateVisibility TargetVisibility = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden;

	if (bVisible)
	{
		ShowAttractVideoParents();
	}

	if (AttractVideoRoot)
	{
		AttractVideoRoot->SetVisibility(TargetVisibility);
	}

	if (AttractVideoImage)
	{
		AttractVideoImage->SetVisibility(bVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Hidden);
	}

	if (AttractInputCatcher)
	{
		AttractInputCatcher->SetVisibility(TargetVisibility);
	}

	if (AttractInputCatcherText)
	{
		if (bVisible)
		{
			AttractTextBlinkElapsed = 0.f;
			AttractInputCatcherText->SetVisibility(ESlateVisibility::HitTestInvisible);
			AttractInputCatcherText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		}
		else
		{
			AttractInputCatcherText->SetVisibility(ESlateVisibility::Hidden);
			AttractInputCatcherText->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.f)));
		}
	}

	if (!bVisible)
	{
		RestoreAttractVideoParents();
	}
}

void UUI_MainMenu::SetMenuButtonsEnabled(bool bEnabled)
{
	if (StartBtn)
	{
		StartBtn->SetIsEnabled(bEnabled);
	}

	if (SetBtn)
	{
		SetBtn->SetIsEnabled(bEnabled);
	}

	if (ExitBtn)
	{
		ExitBtn->SetIsEnabled(bEnabled);
	}
}

void UUI_MainMenu::ShowAttractVideoParents()
{
	if (!AttractVideoImage)
	{
		return;
	}

	UPanelWidget* Parent = AttractVideoImage->GetParent();
	while (Parent)
	{
		if (!SavedAttractParentVisibilities.Contains(Parent))
		{
			SavedAttractParentVisibilities.Add(Parent, Parent->GetVisibility());
		}

		Parent->SetVisibility(ESlateVisibility::Visible);
		Parent = Parent->GetParent();
	}
}

void UUI_MainMenu::RestoreAttractVideoParents()
{
	for (const TPair<TWeakObjectPtr<UWidget>, ESlateVisibility>& SavedVisibility : SavedAttractParentVisibilities)
	{
		if (UWidget* Widget = SavedVisibility.Key.Get())
		{
			Widget->SetVisibility(SavedVisibility.Value);
		}
	}

	SavedAttractParentVisibilities.Reset();
}

bool UUI_MainMenu::CanPlayAttractVideo() const
{
	return AttractVideoSource != nullptr && AttractVideoImage != nullptr;
}

UMediaPlayer* UUI_MainMenu::GetAttractMediaPlayer()
{
	if (AttractMediaPlayer)
	{
		return AttractMediaPlayer;
	}

	if (!RuntimeAttractMediaPlayer)
	{
		RuntimeAttractMediaPlayer = NewObject<UMediaPlayer>(this);
	}

	return RuntimeAttractMediaPlayer;
}

void UUI_MainMenu::PrepareAttractVideoImage()
{
	UMediaPlayer* MediaPlayer = GetAttractMediaPlayer();
	if (!AttractVideoImage || !MediaPlayer)
	{
		return;
	}

	if (!RuntimeAttractMediaTexture)
	{
		RuntimeAttractMediaTexture = NewObject<UMediaTexture>(this);
	}

	if (RuntimeAttractMediaTexture)
	{
		RuntimeAttractMediaTexture->SetMediaPlayer(MediaPlayer);
		RuntimeAttractMediaTexture->UpdateResource();

		FSlateBrush Brush = AttractVideoImage->GetBrush();
		Brush.DrawAs = ESlateBrushDrawType::Image;
		if (Brush.ImageSize.X <= 0.f || Brush.ImageSize.Y <= 0.f)
		{
			Brush.ImageSize = FVector2D(1920.f, 1080.f);
		}
		Brush.SetResourceObject(RuntimeAttractMediaTexture.Get());
		AttractVideoImage->SetBrush(Brush);
	}
}

void UUI_MainMenu::BindSettingsClosed()
{
	if (!SettingsWidget)
	{
		return;
	}

	SettingsWidget->OnSettingsMenuClosed.RemoveDynamic(this, &UUI_MainMenu::HandleSettingsClosed);
	SettingsWidget->OnSettingsMenuClosed.AddDynamic(this, &UUI_MainMenu::HandleSettingsClosed);
}
