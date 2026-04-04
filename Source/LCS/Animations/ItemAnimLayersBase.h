// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ItemAnimLayersBase.generated.h"

class ULCSAnimInstance;

UCLASS()
class LCS_API UItemAnimLayersBase : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	void NativeInitializeAnimation() override;
	void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	
protected:
	void GetMainAnimIstanceThreadSafe();
	
	void UpdateBlendWeightData(float DeltaSeconds);
	void UpdateJumpFallData(float DeltaSeconds);
	void UpdateSkelControlData(float DeltaSeconds);
	
protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category="Refs")
	TWeakObjectPtr<ULCSAnimInstance> MainAnimInstance;
	
	// =========================== 混合参数 ===========================
	UPROPERTY(BlueprintReadOnly, Category="Blend Weight Data")
	float HipFireUpperBodyOverrideWeight;	// 上半身覆盖权重: 决定战斗端枪还是基础跑动
	
	UPROPERTY(BlueprintReadOnly, Category="Blend Weight Data")
	float AimOffsetBlendWeight;				// 瞄准偏移权重：决定角色腰部是否需要跟着鼠标上下左右扭曲
	
	// =========================== 设置参数 ===========================
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings")
	FVector2D PlayRateClampStartsPivots = FVector2D(0.6f, 5.0f);  // 起步和急停播放速率区间
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings")
	FName LocomotionDistanceCurveName = FName(TEXT("Distance"));	// 用于距离匹配的动画曲线名字
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings")
	FVector2D PlayRateClampCycle = FVector2D(0.8f, 1.2f);	// 循环跑动的播放速率限制
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings")
	bool bRaiseWeaponAfterFiringWhenCrouched = false;	// 蹲下是否持枪
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings")
	float RaiseWeaponAfterFiringDuration = 0.5f;	// 开火之后恢复持枪姿势的时间
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings")
	bool bDisableHandIK = false;	// 手部IK开关
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings")
	bool bEnableLeftHandPoseOverride = false;  // 左手姿态独立覆盖开关
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings")
	float StrideWarpingBlendInDurationScaled = 0.2f;	// 扭曲平滑淡入时长
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Settings")
	float StrideWarpingBlendInStartOffset = 0.15f;	// 扭曲介入延迟，防止脚步还没离地就开启扭曲
	
	// ==================================== 跳跃相关数据 ===========================================
	UPROPERTY(BlueprintReadWrite, Category="Jump")
	float TimeFalling = 0.0f;
	
	UPROPERTY(BlueprintReadWrite, Category="Jump")
	float LandRecoveryAlpha = 0.0f;
	
	// =================================== 手部IK相关数据 ==========================================
	UPROPERTY(BlueprintReadWrite, Category="Skel Control Data")
	float HandIKRightAlpha;
	
	UPROPERTY(BlueprintReadWrite, Category="Skel Control Data")
	float HandIKLeftAlpha;
	
	UPROPERTY(BlueprintReadWrite, Category="Skel Control Data")
	float LeftHandPoseOverrideWeight;
	
	UPROPERTY(BlueprintReadWrite, Category="Skel Control Data")
	float HandFKWeight;
	
	// =================================== Stride Warpping =================================== 
	UPROPERTY(BlueprintReadWrite, Category="Stride Warpping")
	float StrideWarpingStartAlpha;
	
	UPROPERTY(BlueprintReadWrite, Category="Stride Warpping")
	float StrideWarpingPivotAlpha;
	
	UPROPERTY(BlueprintReadWrite, Category="Stride Warpping")
	float StrideWarpingCycleAlpha;
	
	// =================================== 原地转身 =================================== 
	UPROPERTY(BlueprintReadWrite, Category="Turn In Place")
	float TurnInPlaceAnimTime;
	
	UPROPERTY(BlueprintReadWrite, Category="Turn In Place")
	float TurnInPlaceRotationDirection;
	
	UPROPERTY(BlueprintReadWrite, Category="Turn In Place")
	float TurnInPlaceRecoveryDirection;
	
	// =================================== Idle breaks =================================== 
	UPROPERTY(BlueprintReadWrite, Category="Idle breaks")
	bool bWantsIdleBreak = false;
	
	UPROPERTY(BlueprintReadWrite, Category="Idle breaks")
	float TimeUntilNextIdleBreak;
	
	UPROPERTY(BlueprintReadWrite, Category="Idle breaks")
	int CurrentIdleBreakIndex;
	
	UPROPERTY(BlueprintReadWrite, Category="Idle breaks")
	float IdleBreakDelayTime;
	
	// // =================================== pivots =================================== 
	UPROPERTY(BlueprintReadWrite, Category="Pivots")
	FVector PivotStartingAcceleration;
	
	UPROPERTY(BlueprintReadWrite, Category="Pivots")
	float TimeAtPivotStop;
};
