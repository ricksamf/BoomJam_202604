# 敌人动画蓝图交接文档

> 本文档作为 `Documents/Enemy/Handoff.md` 的延伸,专门记录敌人动画蓝图(ABP)部分的设计、实现与待办事项。
>
> 引擎:UE 5.6.1
> 工具链:UnrealBridge 插件(编辑器内 Python TCP 桥接,用于脚本化创建 / 修改 ABP 和 BP)
> 范围:`/Game/Enemy/Anims/` 下三个 ABP + 四个敌人 BP 的 AnimClass 绑定 + C++ 侧 Montage 播放钩子

---

## 1. 总览

### 1.1 本次交付物

| 类别 | 路径 / 文件 | 状态 |
|---|---|---|
| AnimBlueprint | `/Game/Enemy/Anims/ABP_MeleeEnemy` | 结构搭好,占位动画就位,clean compile |
| AnimBlueprint | `/Game/Enemy/Anims/ABP_PistolEnemy` | 同上 |
| AnimBlueprint | `/Game/Enemy/Anims/ABP_MachineGunEnemy` | 同上 |
| 敌人 BP 修正 | `BP_MeleeEnemy / BP_GhostMeleeEnemy / BP_PistolEnemy / BP_MachineGunEnemy` 的 `CharacterMesh0.AnimClass` | 全部指向正确 ABP |
| C++ 新增字段 | `UMeleeEnemyDataAsset::AttackMontage`<br>`UPistolEnemyDataAsset::FireMontage`<br>`UMachineGunEnemyDataAsset::BurstMontage` | `TObjectPtr<UAnimMontage>`,`EditAnywhere, BlueprintReadOnly`,留空即不播 |
| C++ 新增方法 | `AMeleeEnemy::PlayAttackMontage()`<br>`APistolEnemy::PlayAttackMontage()`<br>`AMachineGunEnemy::PlayAttackMontage()` | `UFUNCTION(BlueprintCallable)` |
| C++ 调用点 | `FEnemyMeleeSwingTask::EnterState`(Swing 起手)<br>`APistolEnemy::FireProjectile`(每次开枪)<br>`AMachineGunEnemy::FireOneBullet`(每颗子弹) | 已插 `PlayAttackMontage()` |

### 1.2 类 / 资产映射

| 敌人 BP | 敌人类(C++) | DataAsset(C++ / 资产) | AnimBlueprint | SkeletalMesh |
|---|---|---|---|---|
| `BP_MeleeEnemy` | `AMeleeEnemy` | `UMeleeEnemyDataAsset` / `DA_MeleeEnemy_Default` | `ABP_MeleeEnemy` | `SKM_Manny_Simple` |
| `BP_GhostMeleeEnemy` | `AGhostMeleeEnemy` | `UMeleeEnemyDataAsset` / `DA_GhostMeleeEnemy_Default` | `ABP_MeleeEnemy`(共享) | `SKM_Manny_Simple` |
| `BP_PistolEnemy` | `APistolEnemy` | `UPistolEnemyDataAsset` / `DA_PistolEnemy_Default` | `ABP_PistolEnemy` | `SKM_Manny_Simple` |
| `BP_MachineGunEnemy` | `AMachineGunEnemy` | `UMachineGunEnemyDataAsset` / `DA_MachineGunEnemy_Default` | `ABP_MachineGunEnemy` | `SKM_Manny_Simple` |

Ghost 在动画上**完全复用 Melee**(Handoff §5.8 说 Ghost 只是 `DefaultRealmType=Realm` 的 Melee 变体,动画差异不存在)。后续要差异化(漂浮 / 消隐等)再单独拆一份 `ABP_GhostMeleeEnemy`。

### 1.3 骨骼

三个 ABP 全部 `TargetSkeleton = /Game/Characters/Mannequins/Meshes/SK_Mannequin`,与敌人当前 Mesh 保持一致。**美术做的 AS_\* 动画基于 `SK_Biped_Template` 骨骼**,需要美术在该骨骼资产里把 `SK_Mannequin` 加入 `Compatible Skeletons` 列表,才能把 `AS_*` 动画直接拖进现有 ABP 使用。详见 §5。

---

## 2. ABP 结构(三个 ABP 同构)

### 2.1 AnimGraph

```
[Output Pose] ── Result ◄── Pose [Slot 'UpperBody'] ── Source ◄── Pose [StateMachine 'Main States']
```

- `Slot 'UpperBody'`:上半身 Montage 注入点。没 Montage 在跑时**等于透传**(把 StateMachine 的 Pose 直接往上传),所以不会干扰 Locomotion;有 Montage 在跑时用 Montage 的 Pose 盖掉——具体覆盖范围(全身还是上半身)由 **Montage 资产自身的 Slot 配置**决定,ABP 不管。
- **Slot 名必须叫 `UpperBody`**。C++ 侧 `PlayAnimMontage(Data->AttackMontage)` 是按 Montage 自带的 Slot 名路由的,所以 Montage 资产也必须有一个名为 `UpperBody` 的 Slot(见 §6.1)。

### 2.2 State Machine `Main States`

只有两个态:

| State | 默认 | 占位 Sequence(SK_Mannequin 骨骼) | 备注 |
|---|---|---|---|
| `Idle` | 是 | `/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle` | 所有 ABP 共用占位 |
| `Jog` | 否 | Melee: `MF_Unarmed_Jog_Fwd` / Pistol: `MF_Pistol_Jog_Fwd` / MG: `MF_Rifle_Jog_Fwd` | 引擎自带,匹配骨骼可编译 |

**为什么占位不能留空**:UE 的 `Sequence Player` 节点如果 Sequence 字段为空,编译直接报 Fatal(`Sequence Player references an unknown Anim Sequence Base`)。所以 ABP 搭好时必须挂一个同骨骼的 Sequence 资产作"占位",美术到位后在 Details 面板里替换即可。美术的替换流程见 §6.3。

### 2.3 Transition Rules

两条对称的转换规则,由 ABP 成员变量 `Speed`(`float`)驱动:

| Transition | Rule 逻辑 | Crossfade |
|---|---|---|
| `Idle → Jog` | `Speed > 10` | 0.2s |
| `Jog → Idle` | `Speed <= 10` | 0.2s |

实现细节:
- `Rule_Idle_to_Jog` graph:`Get Speed → Greater_DoubleDouble(10.0) → TransitionResult.bCanEnterTransition`
- `Rule_Jog_to_Idle` graph:`Get Speed → LessEqual_DoubleDouble(10.0) → TransitionResult.bCanEnterTransition`

> **坑**:UE 5.6 的 `KismetMathLibrary` 里 Python 侧只识别 `Greater_DoubleDouble` / `LessEqual_DoubleDouble`,不识别 `Greater_FloatFloat`(UE5 已把 float 迁移到 double,老名字虽然还在 C++ 里但 Python `add_call_function_node` 找不到)。bridge 调用时必须用 `Double` 版本。

阈值 `10`:玩家速度单位 cm/s;角色 `WalkSpeed` 默认 200、`ChaseSpeed` 从 DataAsset 读。10 这个值是"几乎静止"的阈值,`RInterpConstantTo` 等微小调整造成的残余速度不会误切到 Jog。

### 2.4 EventGraph(参数驱动)

```
[Event Blueprint Update Animation] ─then─► [Set Speed]
                                              ▲
                                              │ (Speed input)
                                              │
[Try Get Pawn Owner] ─Return─► [GetVelocity] ─Return─► [VSize] ─Return─┘
```

等价 C++ 写法:`Speed = TryGetPawnOwner()->GetVelocity().Size();`,每帧刷一次。

> 这里用的是 `BlueprintUpdateAnimation` 事件(UE 传统入口),**不是** `BlueprintThreadSafeUpdateAnimation`。敌人没有 Fast Path 需求(没挂 Aim Offset / Layered Blend Per Bone 这种重计算节点),单线程够用。要切 Thread Safe 时需要把 `TryGetPawnOwner / GetVelocity` 的调用换成线程安全版本(`GetOwningActor` 等),超出本次范围。

### 2.5 变量

| 变量名 | 类型 | 默认值 | 用途 |
|---|---|---|---|
| `Speed` | `float` | 0.0 | Locomotion 驱动参数,由 EventGraph 每帧刷 |

---

## 3. 与 `ABP_Unarmed`(示例)的关系

示例项目提供了 `/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed`,结构比我们的敌人 ABP 复杂:

- 有 Locomotion / Jump / Fall Loop / Land 四态
- Locomotion 内部还有 Idle / Walk-Run 的子 StateMachine,用 `BS_Idle_Walk_Run`(BlendSpace)做速度混合
- 有完整的 Jump Apex / Jump_Fall_Loop / Jump Recovery Additive 节点

**为什么敌人 ABP 没照搬**:
- 美术交付清单只要求 idle / jog / 攻击三种动画,敌人也没跳 / 跌落 / 落地需求
- 近战敌人的 `PerformDash` 会让 `LaunchCharacter` 短暂进入 Falling 状态,但 Dash 时长 0.35s 内肉眼基本看不到腾空姿态,MVP 阶段不需要 Jump/Fall 态
- Locomotion 用 `BS_Idle_Walk_Run`(BlendSpace)还是"Idle ↔ Jog 两态"对敌人没本质区别(敌人速度是离散的 Walk/Chase 切换,不像玩家有模拟摇杆连续速度)。两态实现简单,美术只需要 1 个 Idle + 1 个 Jog,不用做 BlendSpace

后续如果要加 Walk / Run 分层(比如巡逻 Walk + 追击 Jog),把 Jog 态里的 Sequence Player 换成 `BS_Idle_Walk_Run`(BlendSpace Player)即可,Speed 直接喂给它,ABP 图结构基本不动。

---

## 4. 敌人 BP 的 AnimClass 绑定

四个敌人 BP 的 `CharacterMesh0` 组件上设置:
- `AnimationMode = UseAnimBlueprint`
- `AnimClass` 指向对应 ABP 的 GeneratedClass(`ABP_XxxEnemy_C`)

**重要事实**:`CharacterMesh0` 是从 C++ 基类 `ACharacter` 继承来的组件(`inherited=True`),不能用 bridge 的 `Blueprint.set_component_property` 直接改(会返回 False)。必须走:

```python
bp_asset = unreal.load_asset(bp_path)
cdo = unreal.get_default_object(bp_asset.generated_class())
mesh_comp = cdo.get_editor_property("Mesh")   # ACharacter::Mesh 是 SkeletalMeshComponent,名字 CharacterMesh0
mesh_comp.modify()
mesh_comp.set_editor_property("AnimationMode", unreal.AnimationMode.ANIMATION_BLUEPRINT)
mesh_comp.set_editor_property("AnimClass", anim_gen_cls)
# 编译 + save_loaded_asset
```

修改 CDO 后必须编译 BP 并 save,否则内存修改不会持久化。

### 4.1 修掉的历史错配

修改前,`BP_MeleeEnemy` 和 `BP_GhostMeleeEnemy` 的 `AnimClass` 被错误地指向 `/Game/Player/Characters/ABP_GsPlayer`(**玩家的 ABP**),这是本次一并修正的 bug。Pistol 和 MG 修改前 `AnimClass = None`,敌人仅按骨骼默认 Pose 显示,没有任何动画。

---

## 5. 骨骼兼容性——美术需要做的关键配置

### 5.1 现状

| 动画 | 来源 | TargetSkeleton |
|---|---|---|
| `AS_Player_idle` / `AS_Slash01/02/03` / `AS_Slash` / `AS_PlayerSlash` / `AS_PlayerSlash_Montage` | 美术 | **`SK_Biped_Template`** (`/ControlRigModules/Modules/Meshes/`) |
| `MM_Idle` / `MF_Unarmed_Jog_Fwd` / `MF_Pistol_Jog_Fwd` / `MF_Rifle_Jog_Fwd` / `MM_Pistol_Fire` / `MM_Pistol_Fire_Montage` / `MM_Rifle_Fire` / `FP_Rifle_Shoot_Montage` | 引擎示例 | **`SK_Mannequin`** (`/Game/Characters/Mannequins/Meshes/`) |
| 敌人 Mesh (`SKM_Manny_Simple`) | 引擎示例 | `SK_Mannequin` |
| 本次建的三个 ABP | 我 | `SK_Mannequin` |

### 5.2 问题

一个 ABP 只能挂**同一个 TargetSkeleton 的动画**。美术的 `AS_*` 骨骼和我们 ABP 的骨骼不一致,直接拖到 ABP 的 `Sequence Player` 会被拒绝。

### 5.3 解决方案(美术侧一次性操作)

UE5 提供了 **Compatible Skeletons** 机制,允许一个骨骼声明"我和另一个骨骼结构兼容,我的动画可以跨骨骼使用"。

操作:
1. 打开 `/ControlRigModules/Modules/Meshes/SK_Biped_Template`(这是美术选的那个"模板骨骼")
2. 窗口 → Skeleton → 找到 `Compatible Skeletons` 设置(在 Details 里)
3. 把 `/Game/Characters/Mannequins/Meshes/SK_Mannequin` 加进列表
4. 保存

这样 `AS_Player_idle` / `AS_Slash01` 等就可以直接在 ABP_MeleeEnemy(SK_Mannequin 骨骼)的 Sequence Player 里选到,不需要 Retarget。

> 如果美术由于某种原因不做这个配置,退化方案是在编辑器里用 `IK Retargeter` 把 `AS_*` 重定向生成 SK_Mannequin 版本(更费工),或者把敌人 Mesh 换成 SK_Biped_Template 对应的 mesh(但那会反过来让引擎自带 `MF_*_Jog_Fwd` 不可用)。**推荐走 Compatible Skeletons,这是最干净的路径**。

### 5.4 相反方向

美术配置 Compatible 后,**只是美术动画能在 SK_Mannequin 上用**。反过来(引擎 jog 在 Biped 上用)不需要,我们用不到。

---

## 6. Montage 播放链路

### 6.1 数据流

```
DataAsset(资产层)
  UMeleeEnemyDataAsset::AttackMontage         ◄──── 美术 / 用户手工在编辑器里挂上 AnimMontage 资产
  UPistolEnemyDataAsset::FireMontage
  UMachineGunEnemyDataAsset::BurstMontage
        │
        │ BeginPlay / ApplyDataAsset 时 EnemyData 被读到 Character 里
        ▼
敌人 Character(运行时)
  PlayAttackMontage()  ──── 内部 PlayAnimMontage(Data->XxxMontage)  (ACharacter 基类方法)
        │
        ▼
ABP(AnimGraph)
  Slot 'UpperBody' ◄──── PlayAnimMontage 按 Montage 自带的 Slot 名路由到匹配的 Slot 节点上
        │
        ▼
Output Pose                                   → 最终显示为"跑动中挥砍"或"站立开枪"等动作
```

**关键点**:`PlayAnimMontage(Montage)` 路由到哪个 Slot,看的是 **Montage 资产自身的 Slot Name**,不是 ABP 里 Slot 节点的名字。所以 Montage 的 Slot Name **必须和 ABP 里的 Slot 节点同名**(都是 `UpperBody`),动画才会出现;否则 Montage 播了但没地方挂,屏幕上啥反应都没有。

### 6.2 Montage 资产配置要求

对于要给敌人用的每个 Montage:
- **Target Skeleton**:`SK_Mannequin`(或依赖 §5 的 Compatible Skeletons 设置,底层也是 SK_Mannequin 可识别)
- **Slot 轨道名**:`UpperBody`(默认的 `DefaultSlot` 不行,必须改)

怎么改 Slot Name:
1. 双击打开 Montage 资产
2. 时间轴左侧有一栏 Slot(默认叫 `DefaultSlot`)
3. 点该 Slot → `Slot Name → New Slot` 或选现有的 → 输入 `UpperBody`

### 6.3 从 AnimSequence 创建 Montage

美术可能交付的是 AnimSequence(`AS_Slash01` 等)而不是 Montage。C++ 里 `PlayAnimMontage` 只吃 `UAnimMontage`,不吃 `UAnimSequence`,所以需要一步转换:

1. Content Browser 里右键 `AS_Slash01`
2. `Create → Create AnimMontage`
3. 生成 `AS_Slash01_Montage`,重命名成有意义的名字(`AM_MeleeSwing` 等)
4. 打开该 Montage,按 §6.2 改 Slot Name 为 `UpperBody`
5. 填到对应 DataAsset 的 Montage 字段

### 6.4 三个触发点的行为差异

| Montage | 触发点 | 频率 | 注意 |
|---|---|---|---|
| `AttackMontage`(Melee) | `FEnemyMeleeSwingTask::EnterState` | 每次 Swing 一次(对应 LockOn → Dash → Swing 序列里的 Swing 开始帧) | Montage 时长建议 ≤ `SwingDuration`(`DA_MeleeEnemy_Default.SwingDuration`,默认 0.4s);太长会被下一次 Swing 打断 |
| `FireMontage`(Pistol) | `APistolEnemy::FireProjectile` | 每次开枪一次(Aim → Fire 序列的 Fire 帧) | 时长建议 ≤ `Cooldown`(默认 1.6s),防止下次 Aim 时还在播 |
| `BurstMontage`(MG) | `AMachineGunEnemy::FireOneBullet` | **每颗子弹一次**(Burst 期间按 FireRate ~6/s) | UE 里重复 `PlayAnimMontage` 同一 Montage 会从头重播——如果你希望机枪一个"持续 recoil"效果,Montage 内容应设计成很短的 recoil pulse(0.1~0.2s),每颗子弹抖一下手臂即可 |

MG 每颗子弹都重播的行为是"机枪 recoil"常见做法。如果希望改成"Burst 期间只播一次长 Montage",需要额外状态记录"当前 Burst 内是不是第一颗"——本版不实现,以后需要时在 `AMachineGunEnemy` 里加 bool flag 即可。

### 6.5 三个 Montage 都可留空

`PlayAttackMontage` 里先 `if (!Data || !Data->XxxMontage) return;` — 字段留空时静默跳过,不会崩。适合增量配置(先挂 Melee,Pistol / MG 后续再做)。

---

## 7. 编译要求

### 7.1 C++ 侧(首次接手时必须做)

本次改动在 DataAsset 里**新增了 UPROPERTY**,这种改动 **Live Coding 不支持**(实测返回 `Live coding failed`)。原因:新增 UPROPERTY 改变了 UClass 的 reflection layout,运行时热替换会破坏 CDO 和已序列化的 asset。

正确流程:
1. 关闭 Unreal 编辑器(会断 UnrealBridge)
2. 用 VS 打开 `E:\Repo\BoomJam_202604\UEGameJam.sln`,选 `Development Editor | Win64` 编译
   - 或命令行:
     ```
     "C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" UEGameJamEditor Win64 Development -Project="E:\Repo\BoomJam_202604\UEGameJam.uproject" -WaitMutex
     ```
3. 重开编辑器

### 7.2 ABP 侧

所有三个 ABP 已 clean compile(最终 `get_compile_errors` 只有 Info 级别)。后续美术替换 Sequence / Montage 资产后,**保存即可**,ABP 会自动 auto-compile。

### 7.3 敌人 BP 侧

四个敌人 BP 改了 AnimClass 后都做了 compile + save。**不需要**再次手动编译,除非后续又手改了组件。

---

## 8. 接手后的任务清单

按优先级排(假设你刚重启完编辑器):

### 8.1 最小可跑路径(5 分钟验证链路通)

目的:确认 C++ 编译正确、ABP 挂载正确、敌人 PIE 时有 Locomotion 动画。

1. 打开 `EnemyTestMap.umap`(`/Game/Enemy/EnemyTestMap`)
2. PIE,近战 / 远程敌人应该:
   - 待机时播 `MM_Idle`(占位 idle)
   - 追击时 `Speed > 10`,切到 `Jog`,播 `MF_*_Jog_Fwd`(占位 jog)
3. 如果敌人全程 T-Pose → AnimClass 没挂对或 ABP 编译失败,回查 §4 / §7

### 8.2 挂 Montage 试效果(无需美术,引擎自带资产就能跑)

目的:看到敌人攻击时有动画,而不是只有 Hitbox / 子弹。

1. 打开 `/Game/Enemy/Data/DA_MeleeEnemy_Default`
2. 找 `Melee|Anim → Attack Montage`,挂 `/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_01_Montage`(引擎自带,SK_Mannequin 骨骼,已经是 Montage 格式)
3. **先确认该 Montage 的 Slot Name 是 `UpperBody`**,如果是默认 `DefaultSlot`,改名(见 §6.2)
4. PIE,玩家走到近战敌人前方,敌人 Swing 时应看到挥手动作

对 Pistol / MG 同理:
- Pistol:挂 `/Game/Characters/Mannequins/Anims/Pistol/MM_Pistol_Fire_Montage`
- MG:基于 `MM_Rifle_Fire`(Sequence)创建 Montage 再挂(引擎没自带 TP rifle fire montage),或复用 Pistol Montage

### 8.3 真正的美术资产接入(等美术或自己做)

1. 美术完成 §5.3 的 Compatible Skeletons 配置
2. 用 `AS_Player_idle` 替换三个 ABP 中 Idle 态的 `MM_Idle` 占位:
   - 双击 ABP → 进 AnimGraph → 双击 `Main States` → 双击 `Idle` 态 → 选中 Sequence Player → Details 面板把 `Sequence` 换成 `AS_Player_idle`
3. 同理用美术提供的 jog 动画替换三个 ABP 的 Jog 态占位
4. 按 §6.3 把 `AS_Slash01`(或 03)转成 Montage,改 Slot 为 `UpperBody`,挂到 `DA_MeleeEnemy_Default.AttackMontage`
5. 类似地把 `MM_Pistol_Fire_Montage` 和机枪用的 Rifle 动作挂到远程敌人的 DataAsset

### 8.4 可选增强(本次未做)

- **死亡动画**:目前 `AEnemyCharacter::Die` 直接 Ragdoll。若要 Death Montage,在 `UEnemyDataAsset` 加 `DeathMontage`,`Die` 里先 PlayAnimMontage 再延后 Ragdoll 到 Montage 结束(见 `Handoff.md §7.4`)。
- **HitReact Montage**:在 `AEnemyCharacter::TakeDamage` 里(未死时)播 Montage。
- **Aim Offset**(远程敌人):希望"瞄准玩家时上半身朝玩家"时,在 ABP 里加 `Layered Blend Per Bone` + `Aim Offset Player`(`AO_Pistol` / `AO_Rifle` 已有资产),Pitch/Yaw 由 EventGraph 计算"敌人到玩家的方向向量 - 敌人 Forward"得出。工作量约 1-2 小时。
- **Locomotion 升级**:Jog 态换成 `BlendSpace Player`(用 `BS_Idle_Walk_Run`),Speed 喂进去,就能按速度在 Idle/Walk/Run 之间混合。
- **AnimNotify 驱动 Hitbox**:目前 `SetMeleeHitboxActive(true/false)` 是 Timer 驱动(`HitboxActiveWindow=0.2s` 硬编码窗口)。更精确的做法是在 `AS_Slash01` Montage 里添加 `AN_HitboxOn` / `AN_HitboxOff` AnimNotify,在 C++ 里 `UAnimInstance::OnAnimNotifyBegin` 监听,打开/关闭 Hitbox。这样 Hitbox 完美贴合挥砍关键帧。本次未做,待需要精细对齐时再加。

---

## 9. UnrealBridge 使用要点(踩过的坑)

### 9.1 创建 AnimBlueprint

```python
import unreal
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
skel = unreal.load_asset("/Game/Characters/Mannequins/Meshes/SK_Mannequin")
factory = unreal.AnimBlueprintFactory()
factory.set_editor_property("TargetSkeleton", skel)    # 注意 PascalCase
factory.set_editor_property("ParentClass", unreal.AnimInstance)
new_abp = asset_tools.create_asset(
    asset_name="ABP_XxxEnemy",
    package_path="/Game/Enemy/Anims",
    asset_class=unreal.AnimBlueprint,
    factory=factory,
)
unreal.EditorAssetLibrary.save_loaded_asset(new_abp, only_if_is_dirty=False)
```

- 新建的 ABP 默认 AnimGraph 只有一个 Output Pose 节点(`AnimGraphNode_Root`),默认 EventGraph 已有 `Event Blueprint Update Animation` + `Try Get Pawn Owner`(UE 模板自动放的,直接复用即可)
- 一定要用 `set_editor_property("TargetSkeleton", ...)`,不能 `factory.target_skeleton = ...`(bridge 的 Python binding 走 reflection,要 PascalCase 名字)

### 9.2 连线 Pin 名

| 节点 | Input Pin | Output Pin |
|---|---|---|
| `AnimGraphNode_Root` (Output Pose) | `Result` | (无) |
| `AnimGraphNode_Slot` | `Source` | `Pose` |
| `AnimGraphNode_StateMachine` | (无) | `Pose` |
| `AnimGraphNode_SequencePlayer` | (无) | `Pose` |
| `AnimGraphNode_StateResult`(State 内部) | `Result` | (无) |
| `AnimGraphNode_TransitionResult`(Rule 内部) | `bCanEnterTransition` | (无) |

### 9.3 State 内部 / Transition Rule 内部 Graph 的 Graph Name

```python
Anim.list_anim_graphs(anim_blueprint_path=abp)
# 返回类似:
# AnimGraph   (kind=AnimGraph)
# EventGraph  (kind=Ubergraph)
# Main States (kind=StateMachine, parent=AnimGraph)
# Idle        (kind=State, parent=Main States)
# Jog         (kind=State, parent=Main States)
# Rule_Idle_to_Jog (kind=TransitionRule, parent=Main States)
# Rule_Jog_to_Idle (kind=TransitionRule, parent=Main States)
```

添加 Transition 时 bridge 自动生成 Rule Graph,名字为 `Rule_<From>_to_<To>`。里面默认有一个 `AnimGraphNode_TransitionResult` 节点。

### 9.4 KismetMathLibrary 函数名(UE5 陷阱)

bridge 的 `Blueprint.add_call_function_node(function_name=...)` 需要 **C++ 反射名**(PascalCase),而 `unreal.MathLibrary.xxx` Python 属性是 snake_case。UE 5.6 里数值比较已全部改名:

| 需求 | ✓ 用这个 | ✗ 别用(会静默返回空 guid) |
|---|---|---|
| `a > b`(float) | `Greater_DoubleDouble` | `Greater_FloatFloat` |
| `a < b`(float) | `Less_DoubleDouble` | `Less_FloatFloat` |
| `a >= b` | `GreaterEqual_DoubleDouble` | `GreaterEqual_FloatFloat` |
| `a <= b` | `LessEqual_DoubleDouble` | `LessEqual_FloatFloat` |
| `a == b`(float) | `EqualEqual_DoubleDouble` | `EqualEqual_FloatFloat` |

如果错用 `FloatFloat` 版本,`add_call_function_node` 返回空字符串但**不报错**,连线时才会报 `guid 找不到`。用法:

```python
gt = Blueprint.add_call_function_node(
    blueprint_path=abp, graph_name="Rule_Idle_to_Jog",
    target_class_path="/Script/Engine.KismetMathLibrary",
    function_name="Greater_DoubleDouble",
    node_pos_x=-200, node_pos_y=0,
)
```

### 9.5 继承组件改属性(Inherited Component)

`Blueprint.set_component_property` 对 `inherited=True` 的组件(`CharacterMesh0` 等)**返回 False、改不动**。必须走 CDO 路径:

```python
bp_asset = unreal.load_asset(bp_path)
cdo = unreal.get_default_object(bp_asset.generated_class())
mesh_comp = cdo.get_editor_property("Mesh")
mesh_comp.modify()
mesh_comp.set_editor_property("AnimationMode", unreal.AnimationMode.ANIMATION_BLUEPRINT)
mesh_comp.set_editor_property("AnimClass", anim_gen_cls)
# 然后:Editor.compile_blueprints + save_loaded_asset
```

这个路径会改 CDO,但依赖 `save_loaded_asset` 把 CDO 序列化回 `.uasset`。做完一定要编译 + 保存,否则只活在内存里。

### 9.6 Sequence Player 不能挂空

`Anim.add_anim_graph_node_sequence_player(sequence_path="", ...)` 能**创建**节点,但编译时会报:

```
Error: Sequence Player references an unknown Anim Sequence Base
```

这是 Fatal,整个 ABP 编译失败。必须挂一个同骨骼的占位 Sequence。

### 9.7 Transition Rule 连线看上去成功但编译 Warning

我最初实现时,Pistol / MG 的 rule wire 全部返回 `True`,但编译有 warning:

```
Warning: Idle to Jog will never be taken, please connect something to Can Enter Transition
```

根因:第一次脚本跑到 rule 部分时,TransitionResult 节点刚被 bridge 创建出来,内部 pin 可能还没完全 ready,导致 connect 虽返回 True 但实际没生效。**保险做法**:创建完所有 Transition 后,**单独一个 exec 批次**去拿 TransitionResult 的 GUID 再连线,给 bridge 一次 Tick 把节点初始化完。

本次交付的三个 ABP 已通过重新 wire + 重新编译,`get_compile_errors` 只剩 `Info` 级别。复现步骤可看本文档 git 对应提交的 Python 脚本。

### 9.8 Bash heredoc 和 PowerShell 的限制

在 Claude Code 的 Bash 工具里执行 UnrealBridge Python 脚本时,如果脚本超过某个长度(经验值 ~100 行左右),`bash -c` 会把命令截断,heredoc 终结符找不到导致 `unexpected EOF`。解决办法:
- 拆成多个小批次 exec(bridge 的 Python REPL 持久,变量可以通过 `globals()["_X"] = ...` 跨批次共享)
- 或者用 `exec-file`(但需要写临时文件)

---

## 10. 文件清单

### 10.1 本次新增资产

```
/Game/Enemy/Anims/ABP_MeleeEnemy.uasset
/Game/Enemy/Anims/ABP_PistolEnemy.uasset
/Game/Enemy/Anims/ABP_MachineGunEnemy.uasset
```

### 10.2 本次修改的蓝图资产

```
/Game/Enemy/Blueprints/BP_MeleeEnemy.uasset         (CharacterMesh0.AnimClass)
/Game/Enemy/Blueprints/BP_GhostMeleeEnemy.uasset    (CharacterMesh0.AnimClass)
/Game/Enemy/Blueprints/BP_PistolEnemy.uasset        (CharacterMesh0.AnimClass + AnimationMode)
/Game/Enemy/Blueprints/BP_MachineGunEnemy.uasset    (CharacterMesh0.AnimClass + AnimationMode)
```

### 10.3 本次修改的 C++ 文件

```
Source/UEGameJam/Enemy/Melee/MeleeEnemyDataAsset.h      (+AttackMontage)
Source/UEGameJam/Enemy/Melee/MeleeEnemy.h               (+PlayAttackMontage 声明)
Source/UEGameJam/Enemy/Melee/MeleeEnemy.cpp             (+include AnimMontage.h, +PlayAttackMontage 实现)
Source/UEGameJam/Enemy/Melee/MeleeEnemyTasks.cpp        (Swing EnterState 调 PlayAttackMontage)
Source/UEGameJam/Enemy/Pistol/PistolEnemyDataAsset.h    (+FireMontage, 前向声明)
Source/UEGameJam/Enemy/Pistol/PistolEnemy.h             (+PlayAttackMontage 声明)
Source/UEGameJam/Enemy/Pistol/PistolEnemy.cpp           (+include AnimMontage.h, +PlayAttackMontage, FireProjectile 里调用)
Source/UEGameJam/Enemy/MachineGun/MachineGunEnemyDataAsset.h  (+BurstMontage, 前向声明)
Source/UEGameJam/Enemy/MachineGun/MachineGunEnemy.h     (+PlayAttackMontage 声明)
Source/UEGameJam/Enemy/MachineGun/MachineGunEnemy.cpp   (+include AnimMontage.h, +PlayAttackMontage, FireOneBullet 里调用)
```

### 10.4 未修改但相关

- `/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle.uasset`(Idle 占位)
- `/Game/Characters/Mannequins/Anims/Unarmed/Jog/MF_Unarmed_Jog_Fwd.uasset`(Melee Jog 占位)
- `/Game/Characters/Mannequins/Anims/Pistol/Jog/MF_Pistol_Jog_Fwd.uasset`(Pistol Jog 占位)
- `/Game/Characters/Mannequins/Anims/Rifle/Jog/MF_Rifle_Jog_Fwd.uasset`(MG Jog 占位)

这些只是被 ABP 引用(hard reference),美术替换掉后引用消失,不需要删除,作为后备仍有价值。

---

## 11. 小抄——给下一个 agent 的最短上手路径

1. 读 `Documents/Enemy/Handoff.md` 了解敌人系统整体架构(C++ 类 / StateTree / Realm)
2. 读本文档了解动画部分
3. 打开 `/Game/Enemy/Anims/ABP_MeleeEnemy`,进 AnimGraph 和 Main States 看一眼图结构(印证 §2)
4. 打开 `/Game/Enemy/Data/DA_MeleeEnemy_Default`,找 `Melee|Anim → Attack Montage` 字段,确认类型是 `AnimMontage`
5. 要改 ABP:用 UnrealBridge Python(见 §9 坑表),不要徒手改后 git 冲突
6. 要改 C++:改完关编辑器走完整 UBT 编译(§7.1),不要指望 Live Coding

有疑问时:检查 ABP 是否 clean compile(`Editor.compile_blueprints` + `Blueprint.get_compile_errors` 过一遍),然后检查敌人 BP 的 `CharacterMesh0.AnimClass` 是否正确。90% 的"敌人 T-Pose / 动画不动"都是这两者之一。
