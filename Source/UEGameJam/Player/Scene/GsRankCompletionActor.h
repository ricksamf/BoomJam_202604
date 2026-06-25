// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GsRankCompletionActor.generated.h"

class UUI_Rank;

/**
 * 可放置在场景中的通关排行榜打开器。
 */
UCLASS(Blueprintable)
class UEGAMEJAM_API AGsRankCompletionActor : public AActor
{
	GENERATED_BODY()

public:
	AGsRankCompletionActor();

	/** 通关时结算当前玩家成绩，并打开结算排行榜界面 */
	UFUNCTION(BlueprintCallable, Category="Rank")
	bool OpenCompletionRank();

private:
	UPROPERTY(Transient)
	TObjectPtr<UUI_Rank> RankWidget;
};
