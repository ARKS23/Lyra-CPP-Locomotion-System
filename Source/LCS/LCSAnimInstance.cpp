// Fill out your copyright notice in the Description page of Project Settings.


#include "LCSAnimInstance.h"
#include "LCSCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"


void ULCSAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// 缓存角色和角色移动指针
	if (ALCSCharacter* OwningCharacter = Cast<ALCSCharacter>(GetOwningActor()))
	{
		Character = this->GetCharacter();
		MovementComponent = this->GetMovementComponent();
	}
}

void ULCSAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
	
	if (!Character.IsValid())
	{
		Character = this->GetCharacter();
		return;
	}
	
	if (!MovementComponent.IsValid())
	{
		Character = this->GetCharacter();
		return;
	}

	UpdateLocationData(DeltaSeconds);
	UpdateRotationData(DeltaSeconds);
	UpdateVelocityData(DeltaSeconds);
	UpdateAccelerationData(DeltaSeconds);
	UpdateWallDetection(DeltaSeconds);
	UpdateCharacterStateData(DeltaSeconds);
	UpdateBlendWeightData(DeltaSeconds);
	UpdateRootYawOffset(DeltaSeconds);
	UpdateAimingData(DeltaSeconds);
	UpdateJumpFallData(DeltaSeconds);
	
	bIsFirstUpdate = false;	// 关闭防抽搐锁
}

 UCharacterMovementComponent* ULCSAnimInstance::GetMovementComponent()
{
	if (APawn* PawnOwner = TryGetPawnOwner())
	{
		// 对应蓝图的 GetMovementComponent 和 Cast 节点
		return Cast<UCharacterMovementComponent>(PawnOwner->GetMovementComponent());
	}
	return nullptr;
}

ALCSCharacter* ULCSAnimInstance::GetCharacter()
{
	if (ALCSCharacter* OwningCharacter = Cast<ALCSCharacter>(GetOwningActor()))
	{
		return OwningCharacter;
	}
	return nullptr;
}

bool ULCSAnimInstance::ShouldEnableControlRig()
{
	static FName DisableLegIK = FName(TEXT("DisableLegIK"));
	return GetCurveValue(DisableLegIK) <= 0.0 && !bUseFootPlacement;
}

void ULCSAnimInstance::UpdateLocationData(float DeltaSeconds)
{
	// 距离匹配计算: 离地高度，目标距离等
	FVector LastLocation = WorldLocation;
	WorldLocation = Character->GetActorLocation();
	DisplacementSinceLastUpdate =  (WorldLocation - LastLocation).Size2D();
	DisplacementSpeed = UKismetMathLibrary::SafeDivide(DisplacementSinceLastUpdate, DeltaSeconds);
	
	if (bIsFirstUpdate)	// 第一次更新没有上帧信息
	{
		DisplacementSinceLastUpdate = 0;
		DisplacementSpeed = 0;
	}
}

void ULCSAnimInstance::UpdateRotationData(float DeltaSeconds)
{
	const FRotator NewWorldRotation = Character->GetActorRotation();
	YawDeltaSinceLastUpdate = (NewWorldRotation.Yaw - WorldRotation.Yaw);	// 后续可做边界跳变补丁
	WorldRotation = NewWorldRotation;
	YawDeltaSpeed = UKismetMathLibrary::SafeDivide(YawDeltaSinceLastUpdate, DeltaSeconds);
	
	if (bIsFirstUpdate)
	{
		YawDeltaSinceLastUpdate = 0;
		YawDeltaSpeed = 0;
		AdditiveLeanAngle = 0;
		return;
	}
	
	// 后续考虑升级成参数
	float KValue = 0.0375f;
	if (bIsCrouching || bGameplayTag_IsADS) KValue = 0.025;
	AdditiveLeanAngle = YawDeltaSpeed * KValue;
}

void ULCSAnimInstance::UpdateVelocityData(float DeltaSeconds)
{
	bool bWasMovingLastUpdate = !LocalVelocity2D.IsZero();	// 记录上帧是否移动，防止抖动
	WorldVelocity = Character->GetVelocity();	// 世界绝对速度的角色移动速度
	FVector LocalVelocity3D = WorldRotation.UnrotateVector(WorldVelocity);	// 转成本地移动速度
	LocalVelocity2D = FVector2D(LocalVelocity3D.X, LocalVelocity3D.Y); // 转成二维本地速度取消Z轴的速度
	LocalVelocityDirectionAngle = CalculateDirection(WorldVelocity, WorldRotation);	 // 计算移动角度-180~180
	LocalVelocityDirectionAngleWithOffset = LocalVelocityDirectionAngle - RootYawOffset; // 姿态补偿，如果看风景右转了45读，左腿需要向左补偿45度
	LocalVelocityDirection = SelectCardinalDirectionFromAngle(LocalVelocityDirectionAngleWithOffset, CardinalDirectionDeadZone, LocalVelocityDirection, bWasMovingLastUpdate);
	LocalVelocityDirectionNoOffset = SelectCardinalDirectionFromAngle(LocalVelocityDirectionAngle, CardinalDirectionDeadZone, LocalVelocityDirectionNoOffset, bWasMovingLastUpdate);
	//bHasVelocity = !UKismetMathLibrary::NearlyEqual_FloatFloat(LocalVelocity2D.SquaredLength(), 0.0f, 0.000001);
	bHasVelocity = LocalVelocity2D.SquaredLength() > KINDA_SMALL_NUMBER;
}

void ULCSAnimInstance::UpdateAccelerationData(float DeltaSeconds)
{
	// ==========================================
	// 1. 获取并转换基础加速度
	// 作用：WorldAcceleration 代表玩家摇杆的绝对物理推力。
	// 将其转换为本地 2D 空间，用于基础的动画混合与判定。
	// ==========================================
	FVector WorldAcceleration = MovementComponent->GetCurrentAcceleration();
	FVector LocalAcceleration = WorldRotation.UnrotateVector(WorldAcceleration);
	LocalAcceleration2D = FVector2D(LocalAcceleration.X, LocalAcceleration.Y);
	// 判断当前是否有玩家输入（摇杆是否被推动）
	bHasAcceleration = LocalAcceleration2D.SquaredLength() > KINDA_SMALL_NUMBER;
	
	
	// ==========================================
	// 2. 枢轴 (Pivot) 方向计算与平滑
	// 核心目标：根据加速度计算基准方向，用于触发急转身 (Pivot)
	// 关键原因：速度有物理惯性，而加速度没有。加速度能瞬间反映玩家“想回头”的操作意图。
	// ==========================================
	if (bHasAcceleration)
	{
		// 获取世界加速度 2D 并归一化
		FVector2D WorldAcc2D = FVector2D(WorldAcceleration.X, WorldAcceleration.Y);
		FVector2D NormalizedWorldAcc = WorldAcc2D.GetSafeNormal();

		// Lerp 平滑
		PivotDirection2D = FMath::Lerp(PivotDirection2D, NormalizedWorldAcc, 0.5f);

		// 再次归一化 
		PivotDirection2D.Normalize();

		// Calculate Direction (使用高性能 Atan2 替代矩阵运算)
		FVector LocalPivotDirection3D = WorldRotation.UnrotateVector(FVector(PivotDirection2D.X, PivotDirection2D.Y, 0.0f));
		float PivotDirectionAngle = FMath::RadiansToDegrees(FMath::Atan2(LocalPivotDirection3D.Y, LocalPivotDirection3D.X));

		// 将角度转为四向枚举
		ELCSCardinalDirection TempDirection = SelectCardinalDirectionFromAngle(
			PivotDirectionAngle, 
			CardinalDirectionDeadZone, 
			ELCSCardinalDirection::Forward, // 蓝图中该引脚的值
			false                           // 蓝图中 Use Current Direction 未勾选
		);
		
		CardinalDirectionFromAcceleration = GetOppositeCardinalDirection(TempDirection);
	}
};

void ULCSAnimInstance::UpdateWallDetection(float DeltaSeconds)
{
	/* 角色撞墙检测: 用运动向量数学代替物理射线检测 */
	// 意图检测: 玩家是否有输入
	bool bHasInput = LocalAcceleration2D.SquaredLength() > FMath::Square(0.1f);
	
	// 结果检测: 实际速度是否背压制
	bool bIsSpeedCaped = LocalVelocity2D.SquaredLength() < FMath::Square(200.0f);
	
	// 速度与加速度夹角检测
	bool bIsSlidingAgainstWall = false;
	if (bHasInput && bIsSpeedCaped)
	{
		// 点乘判定夹角
		FVector2D DirAccel = LocalAcceleration2D.GetSafeNormal();
		FVector2D DirVel = LocalVelocity2D.GetSafeNormal();
		float DotResult = FVector2D::DotProduct(DirAccel, DirVel);
		bIsSlidingAgainstWall = (DotResult >= -0.6f && DotResult <= 0.6f); // 夹角范围在大约53° ~ 127°判定是撞墙了
	}
	
	// 撞墙结果
	bIsRunningIntoWall = (bHasInput && bIsSpeedCaped && bIsSlidingAgainstWall);
}

void ULCSAnimInstance::UpdateCharacterStateData(float DeltaSeconds)
{
	// 地面状态设置
	bIsOnGround = MovementComponent->IsMovingOnGround();
	
	// 蹲伏状态设置
	bool bWasCrouchingLastUpdate = bIsCrouching;
	bIsCrouching = MovementComponent->IsCrouching();
	bCrouchStateChange = bIsCrouching != bWasCrouchingLastUpdate;
	
	// 瞄准模式设置
	bADStateChanged = bGameplayTag_IsADS != bWasADSLateUpdate;
	bWasADSLateUpdate = bGameplayTag_IsADS;
	
	// 开火设置
	if (bGameplayTag_IsFiring)
	{
		TimeSinceFiredWeapon = 0.0f;
	}
	else
	{
		TimeSinceFiredWeapon += DeltaSeconds;
	}
	
	// 空中模式设置
	bIsJumping = false;
	bIsFalling = false;
	if (MovementComponent->MovementMode == EMovementMode::MOVE_Falling)
	{
		if (WorldVelocity.Z > 0.0f)	// Z轴方向向上: 跳跃中
		{
			bIsJumping = true;
		}
		else                        // Z轴方向向下: 下坠中
		{
			bIsFalling = true;
		}
	}
}

void ULCSAnimInstance::UpdateBlendWeightData(float DeltaSeconds)
{
	/* 蒙太奇动画混合权重相关 */
	bool bIsPlayingMontage = IsAnyMontagePlaying();	// 是否在播放蒙太奇
	
	if (bIsPlayingMontage && bIsOnGround)
	{
		UpperbodyDynamicAdditiveWeight = 1.0f;	// 地面上播放蒙太奇: 权重拉到1，瞬间接管上半身给玩家及时响应。
	}
	else
	{
		// 蒙太奇结束或者在空中: 平滑退出上半身控制，防止动作抽搐
		UpperbodyDynamicAdditiveWeight = FMath::FInterpTo(UpperbodyDynamicAdditiveWeight, 0.0f, DeltaSeconds, 6.0f);
	}
}

void ULCSAnimInstance::UpdateRootYawOffset(float DeltaSeconds)
{
	// 累加抵消
	if (RootYawOffsetMode == ELCSRootYawOffsetMode::Accumulate)
	{
		float InRootYawOffset = RootYawOffset - YawDeltaSinceLastUpdate;
		SetRootYawOffset(InRootYawOffset);
	}
	
	// BlendOut平滑回正
	if (RootYawOffsetMode == ELCSRootYawOffsetMode::BlendOut || bGameplayTag_IsDashing)
	{
		// 使用底层的弹簧插值算法回复，提升手感
		float InRootYawOffset = UKismetMathLibrary::FloatSpringInterp(
			RootYawOffset, 
			0.0f, 
			RootYawOffsetSpringState, 
			80.0f,  // 刚度越大，弹回得越快
			1.0f,   // 阻尼为 1.0 时是临界阻尼，不会来回震荡过冲
			DeltaSeconds, 
			1.0f,   // 质量，默认 1 即可
			0.5f    // 目标速度衰减
		);
		SetRootYawOffset(InRootYawOffset);
	}
	
	// 下一帧默认衰减
	RootYawOffsetMode = ELCSRootYawOffsetMode::BlendOut;
}

void ULCSAnimInstance::UpdateAimingData(float DeltaSeconds)
{
	// 获取原始的上下瞄准角度，规范化到-180 ~ 180
	float TargetAimPitch = FRotator::NormalizeAxis(Character->GetBaseAimRotation().Pitch);
	// 抗抖: 调整插值速度让枪跟着视角移动的速度改变
	AimPitch = FMath::FInterpTo(AimPitch, TargetAimPitch, DeltaSeconds, 15.0f);
}

void ULCSAnimInstance::UpdateJumpFallData(float DeltaSeconds)
{
	if (bIsJumping)
	{
		if (FMath::IsNearlyZero(MovementComponent->GetGravityZ())) TimeToJumpApex = 0.0f;	// 失重情况
		else TimeToJumpApex = -WorldVelocity.Z / MovementComponent->GetGravityZ();
	}
	else
	{
		TimeToJumpApex = 0.0f;
	}
}

ELCSCardinalDirection ULCSAnimInstance::SelectCardinalDirectionFromAngle(float Angle, float DeadZone,
	ELCSCardinalDirection CurrentDirection, bool bUseCurrentDirection)
{
	float AbsAngle = FMath::Abs(Angle);
	float FwdDeadZone = DeadZone, BwdDeadZone = DeadZone;
	if (bUseCurrentDirection)
	{
		switch (CurrentDirection)
		{
			case ELCSCardinalDirection::Forward:
				FwdDeadZone *= 2;
				break;
			case ELCSCardinalDirection::Backward:
				BwdDeadZone *= 2;
				break;
			default:
				break;
		}
	}
	
	ELCSCardinalDirection result = ELCSCardinalDirection::Forward;
	if (AbsAngle <= 45.f + FwdDeadZone) result = ELCSCardinalDirection::Forward;
	else if (AbsAngle >= 135.f - BwdDeadZone) result = ELCSCardinalDirection::Backward;
	else if (Angle > 0) result = ELCSCardinalDirection::Right;
	else result = ELCSCardinalDirection::Left;
	
	return result;
}

ELCSCardinalDirection ULCSAnimInstance::GetOppositeCardinalDirection(ELCSCardinalDirection CurrentDirection)
{
	switch (CurrentDirection)
	{
		case ELCSCardinalDirection::Forward:
			return ELCSCardinalDirection::Backward;
		case ELCSCardinalDirection::Backward:
			return ELCSCardinalDirection::Forward;
		case ELCSCardinalDirection::Left:
			return ELCSCardinalDirection::Right;
		case ELCSCardinalDirection::Right:
			return ELCSCardinalDirection::Left;
		default:
			return ELCSCardinalDirection::Forward;
	}
}

bool ULCSAnimInstance::IsMovingPerpendicularToInitialPivot()
{
	/* 如果玩家输入突然发生了90度偏移，不再播放转身动画，直接切换到跑动状态 */
	//  判断初始急转身是否在“前后轴 (Y轴)”上
	bool bPivotIsOnFwdBwdAxis = (PivotInitialDirection == ELCSCardinalDirection::Forward || PivotInitialDirection == ELCSCardinalDirection::Backward);
	
	//  判断当前实际速度是否在“前后轴 (Y轴)”上
	bool bVelocityIsOnFwdBwdAxis = (LocalVelocityDirection == ELCSCardinalDirection::Forward || LocalVelocityDirection == ELCSCardinalDirection::Backward);

	//  判断初始急转身是否在“左右轴 (X轴)”上
	bool bPivotIsOnLeftRightAxis = (PivotInitialDirection == ELCSCardinalDirection::Left || PivotInitialDirection == ELCSCardinalDirection::Right);
	
	//  判断当前实际速度是否在“左右轴 (X轴)”上
	bool bVelocityIsOnLeftRightAxis = (LocalVelocityDirection == ELCSCardinalDirection::Left || LocalVelocityDirection == ELCSCardinalDirection::Right);

	//  核心判断：如果它俩不在同一个轴上，就是垂直的！
	return (bPivotIsOnFwdBwdAxis && !bVelocityIsOnFwdBwdAxis) || (bPivotIsOnLeftRightAxis && !bVelocityIsOnLeftRightAxis);
}

void ULCSAnimInstance::SetRootYawOffset(float InRootYawOffset)
{
	if (bEnableRootYawOffset)
	{
		FVector2D ClampVec = bIsCrouching ? RootYawOffsetAngleClampCrouched : RootYawOffsetAngleClamp;
		InRootYawOffset = FRotator::NormalizeAxis(InRootYawOffset); // 规范化角度到 -180 ~ 180
		RootYawOffset = ClampVec.X == ClampVec.Y ? InRootYawOffset : FMath::Clamp(InRootYawOffset, ClampVec.X, ClampVec.Y);
		AimYaw = RootYawOffset * -1.0f; // 上半身永远向着下半身的相反方向扭，从而保证枪口对准摄像机
	}
	else
	{
		// 不开启下半身偏移的情况下，全部归零，身体和胶囊体完全对齐
		RootYawOffset = 0.0f;
		AimYaw = 0.0f;
	}
}

void ULCSAnimInstance::ProcessTurnYawCurve()
{
	float PreviousTurnYawCurveValue = TurnYawCurveValue;
	static FName TurnYawWeight = FName(TEXT("TurnYawWeight"));
	static FName RemainingTurnYaw = FName(TEXT("RemainingTurnYaw"));
	if (FMath::IsNearlyEqual(GetCurveValue(TurnYawWeight), 0.0, 0.0001))
	{
		TurnYawCurveValue = 0.0f;
		PreviousTurnYawCurveValue = 0.0;
	}
	else
	{
		TurnYawCurveValue = GetCurveValue(RemainingTurnYaw) / GetCurveValue(TurnYawWeight);
		if (PreviousTurnYawCurveValue != 0.0)
		{
			float DeltaYaw = PreviousTurnYawCurveValue - TurnYawCurveValue;
			float InTurnYawCurveValue = RootYawOffset - DeltaYaw;
			SetRootYawOffset(InTurnYawCurveValue);
		}
	}
}
