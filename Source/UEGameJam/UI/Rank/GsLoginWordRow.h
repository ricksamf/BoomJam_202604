// Copyright

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GsLoginWordRow.generated.h"

/**
 * 登录界面词库配置行。
 */
USTRUCT(BlueprintType)
struct UEGAMEJAM_API FGsLoginWordRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 随机名字候选，可留空 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Login")
	FString RandomName;

	/** 违禁词，可留空 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Login")
	FString ForbiddenWord;
};
