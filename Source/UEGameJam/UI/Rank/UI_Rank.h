// Copyright

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/Game/GsRankSaveGame.h"
#include "UI_Rank.generated.h"

class UUI_RankPlayerItem;
class UButton;
class UVerticalBox;

/**
 * 排行界面
 */
UCLASS()
class UEGAMEJAM_API UUI_Rank : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Rank")
	void OpenRank();

	UFUNCTION(BlueprintCallable, Category="Rank")
	void OpenSettlementRank();

	UFUNCTION(BlueprintCallable, Category="Rank")
	void RefreshRank();

	UFUNCTION(BlueprintCallable, Category="Rank")
	void SetCurrentPlayerName(const FString& InCurrentPlayerName);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** 排行榜列表容器，需要在 Widget 蓝图中命名为 RankListBox */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> RankListBox;

	/** 回到主界面按钮，结算入口打开排行榜时显示，需要在 Widget 蓝图中命名为 BackMainMenuButton */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> BackMainMenuButton;

	/** 排行榜玩家条目的 Widget 蓝图类 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rank")
	TSubclassOf<UUI_RankPlayerItem> RankPlayerItemClass;

	/** 回到主界面按钮点击后打开的主界面关卡名 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rank")
	FName MainMenuLevelName = TEXT("LEVEL_MainMenu");

	/** 每条排行记录渐变出现的持续时间，单位秒 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rank|Animation", meta=(ClampMin="0.01"))
	float RowAppearDuration = 0.18f;

	/** 排行记录之间依次出现的间隔，单位秒 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rank|Animation", meta=(ClampMin="0.0"))
	float RowAppearInterval = 0.06f;

	/** 排行记录出现前向上偏移的距离 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rank|Animation")
	float RowAppearOffsetY = -24.0f;

	/** 当前玩家条目闪烁的总时长，单位秒 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rank|Animation", meta=(ClampMin="0.0"))
	float CurrentPlayerFlashDuration = 0.6f;

	/** 当前玩家条目在闪烁时的闪动次数 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rank|Animation", meta=(ClampMin="0.0"))
	float CurrentPlayerFlashCount = 2.0f;

	/** 当前玩家条目闪烁时的最低透明度 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rank|Animation", meta=(ClampMin="0.0", ClampMax="1.0"))
	float CurrentPlayerFlashMinOpacity = 0.35f;

private:
	struct FRankAnimatedItem
	{
		TWeakObjectPtr<UUI_RankPlayerItem> Item;
		float StartTime = 0.0f;
		bool bIsCurrentPlayer = false;
	};

	void PlayOpeningAnimation();
	void AddAnimatedItem(UUI_RankPlayerItem* Item, float StartTime, bool bIsCurrentPlayer);
	void FinishOpeningAnimation();
	void SetBackMainMenuButtonVisible(bool bVisible);
	void BuildDisplayRecordIndices(const TArray<FGsRankPlayerRecord>& Records, int32 CurrentPlayerIndex, TArray<int32>& OutIndices) const;
	int32 FindCurrentPlayerIndex(const TArray<FGsRankPlayerRecord>& Records, const FString& PlayerName) const;
	FString GetActiveCurrentPlayerName() const;

	UFUNCTION()
	void HandleBackMainMenuClicked();

	UPROPERTY(Transient)
	TArray<TObjectPtr<UUI_RankPlayerItem>> RankItems;

	TArray<FRankAnimatedItem> AnimatedItems;
	FString ManualCurrentPlayerName;
	float AnimationElapsedTime = 0.0f;
	bool bIsOpeningAnimationPlaying = false;
};
