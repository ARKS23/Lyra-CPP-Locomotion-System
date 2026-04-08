// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Enums/LCSCardinalDirection.h"
#include "Enums/LCSRootYawOffsetMode.h"
#include "Kismet/KismetMathLibrary.h"
#include "LCSAnimInstance.generated.h"

class ALCSCharacter;
class UCharacterMovementComponent;


UCLASS()
class LCS_API ULCSAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	// BeginPlay, 用于缓存指针，避免每帧Cast
	virtual void NativeInitializeAnimation() override;

	// 线程安全的更新动画函数，读取角色状态并更新动画参数
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

protected:
	UFUNCTION(BlueprintPure)
	UCharacterMovementComponent* GetMovementComponent();
	
	UFUNCTION(BlueprintPure)
	ALCSCharacter* GetCharacter();
	
	UFUNCTION(BlueprintPure)
	bool ShouldEnableControlRig();
	
	// ============================== 数据更新函数 ==============================
	UFUNCTION(BlueprintCallable, Category="Update")
	void UpdateLocationData(float DeltaSeconds);
	
	UFUNCTION(BlueprintCallable, Category="Update")
	void UpdateRotationData(float DeltaSeconds);
	
	UFUNCTION(BlueprintCallable, Category="Update")
	void UpdateVelocityData(float DeltaSeconds);
	
	UFUNCTION(BlueprintCallable, Category="Update")
	void UpdateAccelerationData(float DeltaSeconds);
	
	UFUNCTION(BlueprintCallable, Category="Update")
	void UpdateWallDetection(float DeltaSeconds);
	
	UFUNCTION(BlueprintCallable, Category="Update")
	void UpdateCharacterStateData(float DeltaSeconds);
	
	UFUNCTION(BlueprintCallable, Category="Update")
	void UpdateBlendWeightData(float DeltaSeconds);
	
	UFUNCTION(BlueprintCallable, Category="Update")
	void UpdateRootYawOffset(float DeltaSeconds);
	
	UFUNCTION(BlueprintCallable, Category="Update")
	void UpdateAimingData(float DeltaSeconds);
	
	UFUNCTION(BlueprintCallable, Category="Update")
	void UpdateJumpFallData(float DeltaSeconds);
	
	// ============================== 辅助函数 ==============================
	UFUNCTION(BlueprintPure, Category="Helper")
	ELCSCardinalDirection SelectCardinalDirectionFromAngle(float Angle, float DeadZone, ELCSCardinalDirection CurrentDirection, bool bUseCurrentDirection);
	
	UFUNCTION(BlueprintPure, Category="Helper")
	ELCSCardinalDirection GetOppositeCardinalDirection(ELCSCardinalDirection CurrentDirection);
	
	UFUNCTION(BlueprintPure, Category="Helper")
	bool IsMovingPerpendicularToInitialPivot(); // 是否正垂直于初始急转身方向移动,用于急转打断判断
	
	// ============================== 原地转身数据设置辅助函数 ==============================
	UFUNCTION(BlueprintCallable, Category="Turn In Place")
	void SetRootYawOffset(float InRootYawOffset);
	
	UFUNCTION(BlueprintCallable, Category="Turn In Place", meta=(BlueprintThreadSafe))
	void ProcessTurnYawCurve();

public:
	// ============================== 缓存引用 ==============================
    UPROPERTY()
	TWeakObjectPtr<ALCSCharacter> Character;

	UPROPERTY()
	TWeakObjectPtr<UCharacterMovementComponent> MovementComponent;

	
	// ============================== 旋转相关数据 ==============================
	UPROPERTY(BlueprintReadOnly, Category="Rotation Data")
	FRotator WorldRotation;

	UPROPERTY(BlueprintReadOnly, Category="Rotation Data")
	float YawDeltaSinceLastUpdate = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Rotation Data")
	float AdditiveLeanAngle = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Rotation Data")
	float YawDeltaSpeed = 0.0f;

	
	// ============================== 速度相关数据 ==============================
	UPROPERTY(BlueprintReadOnly, Category="Velocity Data")
	FVector WorldVelocity;

	UPROPERTY(BlueprintReadOnly, Category="Velocity Data")
	FVector2D LocalVelocity2D;

	UPROPERTY(BlueprintReadOnly, Category="Velocity Data")
	float LocalVelocityDirectionAngle = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Velocity Data")
	float LocalVelocityDirectionAngleWithOffset = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Velocity Data")
	ELCSCardinalDirection LocalVelocityDirection = ELCSCardinalDirection::Forward;

	UPROPERTY(BlueprintReadOnly, Category="Velocity Data")
	ELCSCardinalDirection LocalVelocityDirectionNoOffset = ELCSCardinalDirection::Forward;

	UPROPERTY(BlueprintReadOnly, Category="Velocity Data")
	bool bHasVelocity = false;
	
	
	// ============================== 加速度数据 ==============================
	UPROPERTY(BlueprintReadOnly, Category="Acceleration Data")
	FVector2D LocalAcceleration2D;
	
	UPROPERTY(BlueprintReadOnly, Category="Acceleration Data")
	bool bHasAcceleration = false;
	
	UPROPERTY(BlueprintReadOnly, Category="Acceleration Data")
	FVector2D PivotDirection2D;
	
	
	// ============================== 角色状态相关 ==============================
	UPROPERTY(BlueprintReadOnly, Category="Character State Data")
	bool bIsOnGround = false;
	
	UPROPERTY(BlueprintReadOnly, Category="Character State Data")
	bool bIsCrouching = false;
	
	UPROPERTY(BlueprintReadOnly, Category="Character State Data")
	bool bIsJumping = false;
	
	UPROPERTY(BlueprintReadOnly, Category="Character State Data")
	bool bCrouchStateChange = false;
	
	UPROPERTY(BlueprintReadOnly, Category="Character State Data")
	bool bADStateChanged = false;
	
	UPROPERTY(BlueprintReadOnly, Category="Character State Data")
	bool bWasADSLateUpdate = false;
	
	UPROPERTY(BlueprintReadOnly, Category="Character State Data")
	float TimeSinceFiredWeapon = 9999.f;
	
	UPROPERTY(BlueprintReadOnly, Category="Character State Data")
	bool bIsFalling = false;
	
	UPROPERTY(BlueprintReadOnly, Category="Character State Data")
	float TimeToJumpApex = 0.f;
	
	UPROPERTY(BlueprintReadOnly, Category="Character State Data")
	bool bIsRunningIntoWall = false;

	
	// ============================== 位置数据: 主要用于距离匹配 ==============================
	UPROPERTY(BlueprintReadOnly, Category="Location Data")
	FVector WorldLocation;

	UPROPERTY(BlueprintReadOnly, Category="Location Data")
	float DisplacementSinceLastUpdate = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Location Data")
	float DisplacementSpeed = 0.0f;
	
	
	// ============================== 标签绑定 ==============================
	UPROPERTY(BlueprintReadOnly, Category="Gameplay Tag Bindings")
	bool bGameplayTag_IsADS = false;
	
	UPROPERTY(BlueprintReadOnly, Category="Gameplay Tag Bindings")
	bool bGameplayTag_IsFiring = false;
	
	UPROPERTY(BlueprintReadOnly, Category="Gameplay Tag Bindings")
	bool bGameplayTag_IsReloading = false;
	
	UPROPERTY(BlueprintReadOnly, Category="Gameplay Tag Bindings")
	bool bGameplayTag_IsDashing = false;
	
	UPROPERTY(BlueprintReadOnly, Category="Gameplay Tag Bindings")
	bool bGameplayTag_IsMelee = false;
	
	
	// ============================== 起步，急转身相关变量 ==============================
	UPROPERTY(BlueprintReadWrite, Category="Locomotion SM Data")
	float LastPivotTime = 0.0f;	// 最后一次转身时间，防抽搐冷却
	
	UPROPERTY(BlueprintReadWrite, Category="Locomotion SM Data")
	ELCSCardinalDirection StartDirection = ELCSCardinalDirection::Forward;	// 起步方向
	
	UPROPERTY(BlueprintReadWrite, Category="Locomotion SM Data")
	ELCSCardinalDirection PivotInitialDirection = ELCSCardinalDirection::Forward;	// 急转身初始方向
	
	UPROPERTY(BlueprintReadOnly, Category="Locomotion SM Data")
	ELCSCardinalDirection CardinalDirectionFromAcceleration = ELCSCardinalDirection::Forward; // 基于加速度的四向方向
	
	UPROPERTY(BlueprintReadOnly, Category="Locomotion SM Data")
	ELCSCardinalDirection LocalAccelerationDirection = ELCSCardinalDirection::Forward; // 加速度方向，用于选择启动动画
	
	
	// ============================== 混合权重数据 ==============================
	UPROPERTY(BlueprintReadOnly, Category="Blend Weight Data")
	float UpperbodyDynamicAdditiveWeight = 0.f;
	
	
	// ============================== 瞄准数据 ==============================
	UPROPERTY(BlueprintReadOnly, Category="Aiming Data")
	float AimPitch = 0.f;
	
	UPROPERTY(BlueprintReadOnly, Category="Aiming Data")
	float AimYaw = 0.f;
	
	
	// ============================== 手柄摇杆角度适配设置 ============================== 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings")
	float CardinalDirectionDeadZone = 10.f;
	
	
	// ============================== 动画Link Layer ==============================
	UPROPERTY(BlueprintReadWrite, Category="Linked Layer Data")
	bool bLinkedLayerChanged = false;
	
	UPROPERTY(BlueprintReadOnly, Category="Linked Layer Data")
	TObjectPtr<UAnimInstance> LastLinkedLayer = nullptr;
	
	
	// ============================== 原地转身相关数据 ==============================
	UPROPERTY(BlueprintReadOnly, Category="Turn In Place Data")
	float RootYawOffset = 0.0f;	// 当前下半身需要反转扭曲的角度
	
	UPROPERTY(BlueprintReadOnly, Category="Turn In Place Data")
	FFloatSpringState RootYawOffsetSpringState;	// 阻尼弹簧防止动作僵硬
	
	UPROPERTY(BlueprintReadWrite, Category="Turn In Place Data")
	float TurnYawCurveValue = 0.0f;	// 转身动画曲线值: 系统每帧读取这个值，用来精准地扣减 RootYawOffset
	
	UPROPERTY(BlueprintReadWrite, Category="Turn In Place Data")
	ELCSRootYawOffsetMode RootYawOffsetMode = ELCSRootYawOffsetMode::BlendOut;	// 偏移模式
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Turn In Place Data")
	FVector2D RootYawOffsetAngleClamp = FVector2D(-90.f, 90.f);	// 定义角色腰最多扭曲的角度
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Turn In Place Data")
	FVector2D RootYawOffsetAngleClampCrouched = FVector2D(-60.f, 60.f);
	
	
	// ============================== 配置项 ==============================
	UPROPERTY(BlueprintReadOnly, Category="Config")
	bool bIsFirstUpdate = true;		// 首次更新标识，防止第一帧插值算法出现扭曲情况
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Config")
	bool bEnableControlRig = true;	// 骨骼控制器开关，距离远的情况下关掉优化
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Config")
	bool bUseFootPlacement = true;	// IK开关，角色在空中或者游泳时避免向下射线检测情况下关闭
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Config")
	bool bEnableRootYawOffset = true;	// 原地转身开关
};
