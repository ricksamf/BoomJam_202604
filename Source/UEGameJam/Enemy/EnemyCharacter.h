// ===================================================
// 文件：EnemyCharacter.h
// 说明：敌人抽象基类。继承 UE 原生 ACharacter（不依赖项目其他角色）。
//       职责：
//         - 挂载 RealmTag / EnemyHealth 组件
//         - 重写 TakeDamage，转发给 Health
//         - Die() 流程：Ragdoll + 关碰撞 + 停 AI + 计时器 Destroy
//         - 向 UEnemySubsystem 注册/注销
//         - ApplyDataAsset() 把 DataAsset 数值拷到运行时
//         - 子类重写 ApplyDataAsset 以处理专属字段
// ===================================================

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Animation/AnimMontage.h"
#include "RealmTagComponent.h"
#include "EnemyCharacter.generated.h"

class URealmTagComponent;
class UEnemyHealthComponent;
class UEnemyDataAsset;
class USoundBase;
class UWidgetComponent;
class AEnemyCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnemyDeathSignature, AEnemyCharacter*, DeadEnemy);

UCLASS(Abstract)
class UEGAMEJAM_API AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent,
	                         AController* EventInstigator, AActor* DamageCauser) override;

	/** 敌人数值资产 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Data")
	TObjectPtr<UEnemyDataAsset> EnemyData;

	/** 默认 RealmType（构造时应用到 RealmTag；蓝图子类可覆写） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy|Realm")
	ERealmType DefaultRealmType = ERealmType::Surface;

	/** 死亡后延迟销毁的时长（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy|Death", meta=(ClampMin=0))
	float DeferredDestructionTime = 3.0f;

	/** Ragdoll 使用的碰撞 Profile 名 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy|Death")
	FName RagdollCollisionProfile = FName("Ragdoll");

	/** 死亡音效列表（每次死亡随机抽一个播放；留空即不播；只填一个则总播这个） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy|Death")
	TArray<TObjectPtr<USoundBase>> DeathSounds;

	/** 重生归属 Checkpoint 编号。复活规则：玩家死亡后，仅当本字段 **严格大于** 玩家当前 CheckpointIndex 时复活。
	 *  -1 = 让 RespawnSubsystem 在 BeginPlay 阶段按"距离最近的 RespawnPoint"自动推断；想精确控制就在编辑器细节面板里填正数。
	 *  典型用法：把 Boss/特殊敌设成下一个段的 Checkpoint 编号，让玩家在当前段死亡不会让 Boss 复活。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|Respawn")
	int32 OwningCheckpointOverride = -1;

	/** RespawnSubsystem 分配的稳定 ID。运行时字段，序列化忽略，重生流程会预设。 */
	UPROPERTY(Transient, BlueprintReadOnly, Category="Enemy|Respawn")
	int32 RespawnRecordId = -1;

	UFUNCTION(BlueprintCallable, Category="Enemy|Respawn")
	void SetRespawnRecordId(int32 NewId) { RespawnRecordId = NewId; }

	/** 敌人死亡广播 */
	UPROPERTY(BlueprintAssignable, Category="Enemy")
	FEnemyDeathSignature OnEnemyDeath;

	/** 是否已死亡 */
	UFUNCTION(BlueprintPure, Category="Enemy")
	bool IsDead() const { return bIsDead; }

	/** 当前 RealmType（从 RealmTag 读取） */
	UFUNCTION(BlueprintPure, Category="Enemy|Realm")
	ERealmType GetEnemyRealmType() const;

	UFUNCTION(BlueprintPure, Category="Enemy|Components")
	URealmTagComponent* GetRealmTag() const { return RealmTag; }

	UFUNCTION(BlueprintPure, Category="Enemy|Components")
	UEnemyHealthComponent* GetHealth() const { return Health; }

	/** 把 DataAsset 的数值拷到运行时属性（基类处理 MaxHP/WalkSpeed 等），子类可扩展 */
	virtual void ApplyDataAsset();

	/** 在指定位置随机播放数组中一个 SoundBase（数组空则什么也不播；只填一个则总播这个）。
	 *  C++ 内部辅助;不暴露蓝图(UFunction 参数不支持 TArray<TObjectPtr<>>)。 */
	void PlayRandomSound(const TArray<TObjectPtr<USoundBase>>& Sounds, const FVector& Location);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy|Components")
	TObjectPtr<URealmTagComponent> RealmTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy|Components")
	TObjectPtr<UEnemyHealthComponent> Health;

	/** 头顶 3D Widget 指示器（Screen 空间,引擎自动 billboard）。Widget Class 由 EnemyData->IndicatorWidgetClass 在 ApplyDataAsset 里注入。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy|UI")
	TObjectPtr<UWidgetComponent> IndicatorWidget;

	UPROPERTY()
	bool bIsDead = false;

	FTimerHandle DeathTimer;

	/** 执行死亡流程：Ragdoll、关碰撞、停 AI、计时器 */
	virtual void Die();

	/** 计时器回调 */
	virtual void DeferredDestruction();

	/** 绑定 Health 的 OnDepleted */
	UFUNCTION()
	void HandleHealthDepleted(UEnemyHealthComponent* Src);

	/** 监听 AnimInstance.OnPlayMontageNotifyBegin —— Montage 时间轴上的 AnimNotify 触发回调。
	 *  目前识别 NotifyName == "Fire" → 调 HandleFireNotify(子类 override 决定开火行为)。
	 *  美术约定:三个敌人的攻击 Montage 在"开火帧"加 NotifyName="Fire"。 */
	UFUNCTION()
	void OnMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& Payload);

	/** "Fire" Notify 触发时的回调,默认空操作。Pistol/MG override 来 spawn 子弹。 */
	virtual void HandleFireNotify() {}
};
