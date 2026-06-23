// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerUI.h"

#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/Widget.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Character/GsPlayer.h"
#include "Player/UI/GsPauseMenuUI.h"
#include "Player/UI/GsRespawnHintUI.h"

void UPlayerUI::BindPlayer(AGsPlayer* InPlayer)
{
	if (BoundPlayer)
	{
		BoundPlayer->OnDeath.RemoveDynamic(this, &UPlayerUI::HandlePlayerDeath);
		BoundPlayer->OnRespawn.RemoveDynamic(this, &UPlayerUI::HandlePlayerRespawn);
		BoundPlayer->OnRespawnHint.RemoveDynamic(this, &UPlayerUI::HandlePlayerRespawnHint);
	}

	BoundPlayer = InPlayer;
	SetDeathWidgetVisible(false);
	UpdateSkillCooldown();
	UpdateDashImage();

	if (!BoundPlayer)
	{
		return;
	}

	BoundPlayer->OnDeath.AddDynamic(this, &UPlayerUI::HandlePlayerDeath);
	BoundPlayer->OnRespawn.AddDynamic(this, &UPlayerUI::HandlePlayerRespawn);
	BoundPlayer->OnRespawnHint.AddDynamic(this, &UPlayerUI::HandlePlayerRespawnHint);
	if (BoundPlayer->IsDead())
	{
		HandlePlayerDeath();
	}
	UpdateSkillCooldown();
	UpdateDashImage();
}

void UPlayerUI::ShowPauseMenu()
{
	if (PauseWidget)
	{
		NewPauseWidget = CreateWidget<UGsPauseMenuUI>(UGameplayStatics::GetPlayerController(GetWorld(), 0), PauseWidget);
		NewPauseWidget->AddToViewport(10);
		
		NewPauseWidget->ShowPauseMenu();
	}
}

void UPlayerUI::HidePauseMenu()
{
	if (NewPauseWidget)
	{
		NewPauseWidget->RemoveFromParent();
		NewPauseWidget = nullptr;
	}
}

void UPlayerUI::TogglePauseMenu()
{
	if (NewPauseWidget)
	{
		HidePauseMenu();
	}
	else
	{
		ShowPauseMenu();
	}
}

void UPlayerUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateSkillCooldown();
	UpdateDashImage();
}

void UPlayerUI::NativeDestruct()
{
	if (NewPauseWidget)
	{
		NewPauseWidget->RemoveFromParent();
		NewPauseWidget = nullptr;
	}

	if (RespawnHintWidget)
	{
		RespawnHintWidget->RemoveFromParent();
		RespawnHintWidget = nullptr;
	}

	if (BoundPlayer)
	{
		BoundPlayer->OnDeath.RemoveDynamic(this, &UPlayerUI::HandlePlayerDeath);
		BoundPlayer->OnRespawn.RemoveDynamic(this, &UPlayerUI::HandlePlayerRespawn);
		BoundPlayer->OnRespawnHint.RemoveDynamic(this, &UPlayerUI::HandlePlayerRespawnHint);
		BoundPlayer = nullptr;
	}

	Super::NativeDestruct();
}

void UPlayerUI::HandlePlayerDeath()
{
	SetDeathWidgetVisible(true);
}

void UPlayerUI::HandlePlayerRespawn()
{
	SetDeathWidgetVisible(false);
}

void UPlayerUI::HandlePlayerRespawnHint(const FText& HintText)
{
	ShowRespawnHint(HintText);
}

void UPlayerUI::SetDeathWidgetVisible(bool bVisible)
{
	if (DeathWidget)
	{
		DeathWidget->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void UPlayerUI::UpdateSkillCooldown()
{
	if (SkillCd)
	{
		SkillCd->SetPercent(BoundPlayer ? BoundPlayer->GetSkillCooldownPercent() : 1.0f);
	}
}

void UPlayerUI::UpdateDashImage()
{
	if (DashImg)
	{
		DashImg->SetVisibility(BoundPlayer && BoundPlayer->IsDashAvailable() ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void UPlayerUI::ShowRespawnHint(const FText& HintText)
{
	if (HintText.IsEmpty() || !RespawnHintWidgetClass)
	{
		return;
	}

	if (!RespawnHintWidget)
	{
		if (!GetOwningPlayer())
		{
			return;
		}

		RespawnHintWidget = CreateWidget<UGsRespawnHintUI>(GetOwningPlayer(), RespawnHintWidgetClass);
	}

	if (RespawnHintWidget && !RespawnHintWidget->IsInViewport())
	{
		RespawnHintWidget->AddToPlayerScreen(1);
	}

	if (RespawnHintWidget)
	{
		RespawnHintWidget->SetVisibility(ESlateVisibility::Visible);
		RespawnHintWidget->SetHintText(HintText);
	}
}
