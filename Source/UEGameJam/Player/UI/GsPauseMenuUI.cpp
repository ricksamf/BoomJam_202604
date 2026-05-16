// Copyright Epic Games, Inc. All Rights Reserved.

#include "GsPauseMenuUI.h"

#include "Components/Button.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

static const FName MainMenuLevelName = TEXT("LEVEL_MainMenu");

void UGsPauseMenuUI::ShowPauseMenu()
{
	bIsPauseMenuVisible = true;
	SetVisibility(ESlateVisibility::Visible);

	APlayerController* PlayerController = GetOwningPlayer();
	if (PlayerController)
	{
		PlayerController->bShowMouseCursor = true;
		PlayerController->SetIgnoreMoveInput(true);
		PlayerController->SetIgnoreLookInput(true);

		FInputModeUIOnly InputMode;
		if (ResumeButton)
		{
			InputMode.SetWidgetToFocus(ResumeButton->TakeWidget());
		}
		PlayerController->SetInputMode(InputMode);
	}

	UGameplayStatics::SetGamePaused(this, true);
}

void UGsPauseMenuUI::HidePauseMenu()
{
	if (!bIsPauseMenuVisible)
	{
		return;
	}

	bIsPauseMenuVisible = false;
	SetVisibility(ESlateVisibility::Hidden);
	UGameplayStatics::SetGamePaused(this, false);

	APlayerController* PlayerController = GetOwningPlayer();
	if (PlayerController)
	{
		PlayerController->bShowMouseCursor = false;
		PlayerController->SetIgnoreMoveInput(false);
		PlayerController->SetIgnoreLookInput(false);

		FInputModeGameOnly InputMode;
		PlayerController->SetInputMode(InputMode);
	}
}

void UGsPauseMenuUI::TogglePauseMenu()
{
	if (bIsPauseMenuVisible)
	{
		HidePauseMenu();
	}
	else
	{
		ShowPauseMenu();
	}
}

void UGsPauseMenuUI::NativeConstruct()
{
	Super::NativeConstruct();

	bIsPauseMenuVisible = true;
	//SetVisibility(ESlateVisibility::Hidden);

	if (ResumeButton)
	{
		ResumeButton->OnClicked.RemoveDynamic(this, &UGsPauseMenuUI::HandleResumeClicked);
		ResumeButton->OnClicked.AddDynamic(this, &UGsPauseMenuUI::HandleResumeClicked);
	}

	if (ReturnMainMenuButton)
	{
		ReturnMainMenuButton->OnClicked.RemoveDynamic(this, &UGsPauseMenuUI::HandleReturnMainMenuClicked);
		ReturnMainMenuButton->OnClicked.AddDynamic(this, &UGsPauseMenuUI::HandleReturnMainMenuClicked);
	}
}

void UGsPauseMenuUI::NativeDestruct()
{
	if (bIsPauseMenuVisible)
	{
		HidePauseMenu();
	}

	Super::NativeDestruct();
}

void UGsPauseMenuUI::HandleResumeClicked()
{
	HidePauseMenu();
}

void UGsPauseMenuUI::HandleReturnMainMenuClicked()
{
	HidePauseMenu();
	UGameplayStatics::OpenLevel(this, MainMenuLevelName);
}
