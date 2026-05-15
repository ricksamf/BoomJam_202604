// Copyright

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI_SettingsWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSettingsSelectionChangedSignature, int32, NewIndex, const FText&, NewText);

/**
 * 设置条
 */
UCLASS()
class UEGAMEJAM_API UUI_SettingsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 当前选项改变时广播，NewIndex是新的选项下标，NewText是新的选项文字 */
	UPROPERTY(BlueprintAssignable, Category="Settings Widget")
	FSettingsSelectionChangedSignature OnSelectionChanged;

	/** 初始化设置条显示的描述文字、可选项和默认选项下标 */
	UFUNCTION(BlueprintCallable, Category="Settings Widget")
	void InitializeOptions(const FText& InDescription, const TArray<FText>& InOptions, int32 InInitialIndex);

	/** 设置当前选项下标，bBroadcast为true时会广播选项改变事件 */
	UFUNCTION(BlueprintCallable, Category="Settings Widget")
	void SetCurrentIndex(int32 InIndex, bool bBroadcast);

	/** 获取当前选项下标，未选择时返回INDEX_NONE */
	UFUNCTION(BlueprintPure, Category="Settings Widget")
	int32 GetCurrentIndex() const;

	/** 获取当前选项文字，没有有效选项时返回空文字 */
	UFUNCTION(BlueprintPure, Category="Settings Widget")
	FText GetCurrentOptionText() const;

protected:
	virtual void NativeConstruct() override;

	/** 左侧设置描述文字 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> DescriptionText;

	/** 向左切换选项按钮 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> LeftBtn;

	/** 当前选项显示文字 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> CurrentText;

	/** 向右切换选项按钮 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> RightBtn;

private:
	UFUNCTION()
	void HandleLeftClicked();

	UFUNCTION()
	void HandleRightClicked();

	void UpdateTexts() const;
	bool IsValidCurrentIndex() const;

	UPROPERTY(Transient)
	FText Description;

	UPROPERTY(Transient)
	TArray<FText> Options;

	UPROPERTY(Transient)
	int32 CurrentIndex = INDEX_NONE;
};
