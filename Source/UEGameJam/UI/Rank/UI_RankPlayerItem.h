// Copyright

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/Game/GsRankSaveGame.h"
#include "UI_RankPlayerItem.generated.h"

class UTextBlock;

/**
 * 排行榜玩家条目。
 */
UCLASS()
class UEGAMEJAM_API UUI_RankPlayerItem : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Rank")
	void SetupRankItem(int32 Rank, const FGsRankPlayerRecord& Record, bool bIsCurrentPlayer);

	UFUNCTION(BlueprintPure, Category="Rank")
	bool IsCurrentPlayerItem() const { return bIsCurrentPlayerItem; }

protected:
	/** 当前玩家条目的渲染缩放，用于突出显示当前玩家成绩 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rank|Style", meta=(ClampMin="1.0"))
	float CurrentPlayerScale = 1.12f;

	/** 普通玩家条目的渲染缩放 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rank|Style", meta=(ClampMin="0.1"))
	float NormalPlayerScale = 1.0f;

	/** 排名文本，需要在 Widget 蓝图中命名为 RankText */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> RankText;

	/** 玩家名字文本，需要在 Widget 蓝图中命名为 NameText */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> NameText;

	/** 用时文本，需要在 Widget 蓝图中命名为 TimeText */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> TimeText;

	/** 击杀数文本，需要在 Widget 蓝图中命名为 KillText */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> KillText;

	/** 结束原因文本，需要在 Widget 蓝图中命名为 ReasonText */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> ReasonText;

private:
	static FText FormatElapsedTime(int32 ElapsedMilliseconds);
	static FText GetSettleReasonText(EGsRankSettleReason Reason);

	UPROPERTY(Transient)
	bool bIsCurrentPlayerItem = false;
};
