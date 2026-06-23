// Copyright

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI_Login.generated.h"

class UButton;
class UEditableTextBox;
class UTextBlock;

/**
 * 登录界面
 */
UCLASS()
class UEGAMEJAM_API UUI_Login : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetStartLevelName(FName InStartLevelName);

protected:
	virtual void NativeConstruct() override;

	/** 玩家名字输入框 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> NameInputBox;

	/** 随机生成一个可用名字的按钮 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> RandomNameBtn;

	/** 确认登录按钮，名字可用时才可点击 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> ConfirmBtn;

	/** 名字可用性提示文字 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> HintText;

	/** 登录成功后进入的关卡名 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Login")
	FName StartLevelName = TEXT("Level_NoWhiteBox");

private:
	UFUNCTION()
	void HandleNameTextChanged(const FText& NewText);

	UFUNCTION()
	void HandleRandomNameClicked();

	UFUNCTION()
	void HandleConfirmClicked();

	void LoadWordLists();
	void UpdateLoginState();
	bool IsNameAvailable(const FString& Name, FText& OutHint) const;
	FString GetTrimmedInputName() const;
	FString GenerateAvailableRandomName() const;

	UPROPERTY(Transient)
	TArray<FString> RandomNames;

	UPROPERTY(Transient)
	TArray<FString> ForbiddenWords;
};
