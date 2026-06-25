// Copyright

#include "UI_Login.h"

#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Player/Game/GsRankSaveGame.h"
#include "Player/Game/GsRankRunSubsystem.h"
#include "Settings/GsProjectResourceSettings.h"
#include "UI/Rank/GsLoginWordRow.h"

static constexpr int32 MaxLoginNameLength = 12;
static constexpr int32 MaxRandomNameSuffix = 9999;

static FString GetNormalizedLoginName(const FString& Name)
{
	return Name.TrimStartAndEnd().ToLower();
}

static FString BuildLoginNameWithSuffix(const FString& BaseName, int32 Suffix)
{
	const FString SuffixText = FString::FromInt(Suffix);
	const int32 BaseNameMaxLength = MaxLoginNameLength - SuffixText.Len();
	if (BaseNameMaxLength <= 0)
	{
		return FString();
	}

	const FString TrimmedBaseName = BaseName.TrimStartAndEnd();
	if (TrimmedBaseName.IsEmpty())
	{
		return FString();
	}

	return FString::Printf(TEXT("%s%s"), *TrimmedBaseName.Left(BaseNameMaxLength), *SuffixText);
}

void UUI_Login::SetStartLevelName(FName InStartLevelName)
{
	StartLevelName = InStartLevelName;
}

void UUI_Login::NativeConstruct()
{
	Super::NativeConstruct();

	LoadWordLists();

	if (NameInputBox)
	{
		NameInputBox->OnTextChanged.RemoveDynamic(this, &UUI_Login::HandleNameTextChanged);
		NameInputBox->OnTextChanged.AddDynamic(this, &UUI_Login::HandleNameTextChanged);
	}

	if (RandomNameBtn)
	{
		RandomNameBtn->OnClicked.RemoveDynamic(this, &UUI_Login::HandleRandomNameClicked);
		RandomNameBtn->OnClicked.AddDynamic(this, &UUI_Login::HandleRandomNameClicked);
	}

	if (ConfirmBtn)
	{
		ConfirmBtn->OnClicked.RemoveDynamic(this, &UUI_Login::HandleConfirmClicked);
		ConfirmBtn->OnClicked.AddDynamic(this, &UUI_Login::HandleConfirmClicked);
	}

	if (BackBtn)
	{
		BackBtn->OnClicked.RemoveDynamic(this, &UUI_Login::HandleBackClicked);
		BackBtn->OnClicked.AddDynamic(this, &UUI_Login::HandleBackClicked);
	}

	UpdateLoginState();
}

void UUI_Login::HandleNameTextChanged(const FText& NewText)
{
	static_cast<void>(NewText);

	UpdateLoginState();
}

void UUI_Login::HandleRandomNameClicked()
{
	const FString RandomName = GenerateAvailableRandomName();
	if (RandomName.IsEmpty())
	{
		if (HintText)
		{
			HintText->SetText(FText::FromString(TEXT("没有可用随机名字")));
		}

		return;
	}

	if (NameInputBox)
	{
		NameInputBox->SetText(FText::FromString(RandomName));
	}

	UpdateLoginState();
}

void UUI_Login::HandleConfirmClicked()
{
	const FString PlayerName = GetTrimmedInputName();

	FText Hint;
	if (!IsNameAvailable(PlayerName, Hint))
	{
		UpdateLoginState();
		return;
	}

	UGsRankRunSubsystem* RankRunSubsystem = UGsRankRunSubsystem::Get(this);
	if (!RankRunSubsystem || !RankRunSubsystem->StartRun(PlayerName))
	{
		if (HintText)
		{
			HintText->SetText(FText::FromString(TEXT("开始记录失败")));
		}

		return;
	}

	if (!StartLevelName.IsNone())
	{
		UGameplayStatics::OpenLevel(this, StartLevelName);
	}
}

void UUI_Login::HandleBackClicked()
{
	RemoveFromParent();
	OnLoginClosed.Broadcast();
}

void UUI_Login::LoadWordLists()
{
	RandomNames.Reset();
	ForbiddenWords.Reset();

	const UGsProjectResourceSettings* ResourceSettings = GetDefault<UGsProjectResourceSettings>();
	UDataTable* LoginWordTable = ResourceSettings ? ResourceSettings->LoginWordTable.LoadSynchronous() : nullptr;
	if (!LoginWordTable)
	{
		return;
	}

	static const FString ContextString(TEXT("LoginWordTable"));
	TArray<FGsLoginWordRow*> WordRows;
	LoginWordTable->GetAllRows(ContextString, WordRows);

	for (const FGsLoginWordRow* WordRow : WordRows)
	{
		if (!WordRow)
		{
			continue;
		}

		const FString RandomName = WordRow->RandomName.TrimStartAndEnd();
		if (!RandomName.IsEmpty())
		{
			RandomNames.Add(RandomName);
		}

		const FString ForbiddenWord = WordRow->ForbiddenWord.TrimStartAndEnd();
		if (!ForbiddenWord.IsEmpty())
		{
			ForbiddenWords.Add(ForbiddenWord);
		}
	}
}

void UUI_Login::UpdateLoginState()
{
	FText Hint;
	const bool bCanConfirm = IsNameAvailable(GetTrimmedInputName(), Hint);

	if (ConfirmBtn)
	{
		ConfirmBtn->SetIsEnabled(bCanConfirm);
	}

	if (HintText)
	{
		HintText->SetText(Hint);
	}
}

bool UUI_Login::IsNameAvailable(const FString& Name, FText& OutHint) const
{
	const FString TrimmedName = Name.TrimStartAndEnd();
	if (TrimmedName.IsEmpty())
	{
		OutHint = FText::FromString(TEXT("名字不能为空"));
		return false;
	}

	if (TrimmedName.Len() > MaxLoginNameLength)
	{
		OutHint = FText::FromString(TEXT("名字不能超过12个字符"));
		return false;
	}

	const FString NormalizedName = GetNormalizedLoginName(TrimmedName);
	for (const FString& ForbiddenWord : ForbiddenWords)
	{
		if (!ForbiddenWord.IsEmpty() && NormalizedName.Contains(GetNormalizedLoginName(ForbiddenWord)))
		{
			OutHint = FText::FromString(TEXT("名字包含不可用词"));
			return false;
		}
	}

	if (const UGsRankSaveGame* RankSaveGame = UGsRankSaveGame::LoadOrCreate())
	{
		if (RankSaveGame->ContainsPlayerName(TrimmedName))
		{
			OutHint = FText::FromString(TEXT("名字已存在"));
			return false;
		}
	}

	OutHint = FText::FromString(TEXT("名字可用"));
	return true;
}

FString UUI_Login::GetTrimmedInputName() const
{
	if (!NameInputBox)
	{
		return FString();
	}

	return NameInputBox->GetText().ToString().TrimStartAndEnd();
}

FString UUI_Login::GenerateAvailableRandomName() const
{
	if (RandomNames.Num() <= 0)
	{
		return FString();
	}

	TArray<FString> CandidateNames = RandomNames;
	for (int32 Index = 0; Index < CandidateNames.Num(); ++Index)
	{
		const int32 SwapIndex = FMath::RandRange(Index, CandidateNames.Num() - 1);
		CandidateNames.Swap(Index, SwapIndex);
	}

	FText Hint;
	for (const FString& CandidateName : CandidateNames)
	{
		if (IsNameAvailable(CandidateName, Hint))
		{
			return CandidateName.TrimStartAndEnd();
		}
	}

	for (int32 Suffix = 1; Suffix <= MaxRandomNameSuffix; ++Suffix)
	{
		for (const FString& BaseName : CandidateNames)
		{
			const FString CandidateName = BuildLoginNameWithSuffix(BaseName, Suffix);
			if (IsNameAvailable(CandidateName, Hint))
			{
				return CandidateName;
			}
		}
	}

	return FString();
}
