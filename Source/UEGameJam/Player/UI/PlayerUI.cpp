// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerUI.h"

#include "Components/ProgressBar.h"
#include "Components/Widget.h"
#include "Player/Character/GsPlayer.h"
#include "Player/UI/GsPauseMenuUI.h"

void UPlayerUI::BindPlayer(AGsPlayer* InPlayer)
{
	if (BoundPlayer)
	{
		BoundPlayer->OnDeath.RemoveDynamic(this, &UPlayerUI::HandlePlayerDeath);
		BoundPlayer->OnRespawn.RemoveDynamic(this, &UPlayerUI::HandlePlayerRespawn);
	}

	BoundPlayer = InPlayer;
	SetDeathWidgetVisible(false);
	UpdateSkillCooldown();

	if (!BoundPlayer)
	{
		return;
	}

	BoundPlayer->OnDeath.AddDynamic(this, &UPlayerUI::HandlePlayerDeath);
	BoundPlayer->OnRespawn.AddDynamic(this, &UPlayerUI::HandlePlayerRespawn);
	if (BoundPlayer->IsDead())
	{
		HandlePlayerDeath();
	}
	UpdateSkillCooldown();
}

void UPlayerUI::ShowPauseMenu()
{
	if (PauseWidget)
	{
		PauseWidget->ShowPauseMenu();
	}
}

void UPlayerUI::HidePauseMenu()
{
	if (PauseWidget)
	{
		PauseWidget->HidePauseMenu();
	}
}

void UPlayerUI::TogglePauseMenu()
{
	if (PauseWidget)
	{
		PauseWidget->TogglePauseMenu();
	}
}

void UPlayerUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateSkillCooldown();
}

void UPlayerUI::NativeDestruct()
{
	if (PauseWidget)
	{
		PauseWidget->HidePauseMenu();
	}

	if (BoundPlayer)
	{
		BoundPlayer->OnDeath.RemoveDynamic(this, &UPlayerUI::HandlePlayerDeath);
		BoundPlayer->OnRespawn.RemoveDynamic(this, &UPlayerUI::HandlePlayerRespawn);
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
