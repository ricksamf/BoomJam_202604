// Copyright


#include "UI_SettingsWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UUI_SettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (LeftBtn)
	{
		LeftBtn->OnClicked.RemoveDynamic(this, &UUI_SettingsWidget::HandleLeftClicked);
		LeftBtn->OnClicked.AddDynamic(this, &UUI_SettingsWidget::HandleLeftClicked);
	}

	if (RightBtn)
	{
		RightBtn->OnClicked.RemoveDynamic(this, &UUI_SettingsWidget::HandleRightClicked);
		RightBtn->OnClicked.AddDynamic(this, &UUI_SettingsWidget::HandleRightClicked);
	}

	UpdateTexts();
}

void UUI_SettingsWidget::InitializeOptions(const FText& InDescription, const TArray<FText>& InOptions, int32 InInitialIndex)
{
	Description = InDescription;
	Options = InOptions;

	if (Options.Num() <= 0)
	{
		CurrentIndex = INDEX_NONE;
	}
	else
	{
		CurrentIndex = FMath::Clamp(InInitialIndex, 0, Options.Num() - 1);
	}

	UpdateTexts();
}

void UUI_SettingsWidget::SetCurrentIndex(int32 InIndex, bool bBroadcast)
{
	if (Options.Num() <= 0)
	{
		CurrentIndex = INDEX_NONE;
		UpdateTexts();

		if (bBroadcast)
		{
			OnSelectionChanged.Broadcast(CurrentIndex, GetCurrentOptionText());
		}

		return;
	}

	CurrentIndex = FMath::Clamp(InIndex, 0, Options.Num() - 1);
	UpdateTexts();

	if (bBroadcast)
	{
		OnSelectionChanged.Broadcast(CurrentIndex, GetCurrentOptionText());
	}
}

int32 UUI_SettingsWidget::GetCurrentIndex() const
{
	return CurrentIndex;
}

FText UUI_SettingsWidget::GetCurrentOptionText() const
{
	if (!IsValidCurrentIndex())
	{
		return FText::GetEmpty();
	}

	return Options[CurrentIndex];
}

void UUI_SettingsWidget::HandleLeftClicked()
{
	if (Options.Num() <= 1)
	{
		return;
	}

	const int32 NextIndex = IsValidCurrentIndex() ? (CurrentIndex - 1 + Options.Num()) % Options.Num() : 0;
	SetCurrentIndex(NextIndex, true);
}

void UUI_SettingsWidget::HandleRightClicked()
{
	if (Options.Num() <= 1)
	{
		return;
	}

	const int32 NextIndex = IsValidCurrentIndex() ? (CurrentIndex + 1) % Options.Num() : 0;
	SetCurrentIndex(NextIndex, true);
}

void UUI_SettingsWidget::UpdateTexts() const
{
	if (DescriptionText)
	{
		DescriptionText->SetText(Description);
	}

	if (CurrentText)
	{
		CurrentText->SetText(GetCurrentOptionText());
	}
}

bool UUI_SettingsWidget::IsValidCurrentIndex() const
{
	return Options.IsValidIndex(CurrentIndex);
}
