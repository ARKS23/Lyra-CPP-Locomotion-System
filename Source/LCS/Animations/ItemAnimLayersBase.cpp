// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemAnimLayersBase.h"

#include "LCS/LCSAnimInstance.h"


void UItemAnimLayersBase::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	this->GetMainAnimIstanceThreadSafe();
}

void UItemAnimLayersBase::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
	
	if (!MainAnimInstance.IsValid())
	{
		GetMainAnimIstanceThreadSafe();
		return;
	}
	
	UpdateBlendWeightData(DeltaSeconds);
	UpdateJumpFallData(DeltaSeconds);
	UpdateSkelControlData(DeltaSeconds);
}

void UItemAnimLayersBase::GetMainAnimIstanceThreadSafe()
{
	if (AActor* OwningActor = GetOwningActor())
	{
		// 获取主骨骼的网格体组件，拿到动画实例
		if (USkeletalMeshComponent* MeshComponent = OwningActor->FindComponentByClass<USkeletalMeshComponent>())
		{
			MainAnimInstance = Cast<ULCSAnimInstance>(MeshComponent->GetAnimInstance());
		}
	}
}

void UItemAnimLayersBase::UpdateBlendWeightData(float DeltaSeconds)
{
	bool bIsCrouching = MainAnimInstance->bIsCrouching;
	bool bGameplayTagIsADS = MainAnimInstance->bGameplayTag_IsADS;
	bool bIsOnGround = MainAnimInstance->bIsOnGround;
	
	bool bNormalAimWhileCrouching = !bRaiseWeaponAfterFiringWhenCrouched && bIsCrouching; // 下蹲时且不要求开火后抬枪
	bool bNormalAimWhileStanding = !bIsCrouching && bGameplayTagIsADS && bIsOnGround; // 在地面上处于开镜状态且没有下蹲
	if (bNormalAimWhileCrouching || bNormalAimWhileStanding)
	{
		// 保持标准瞄准
		HipFireUpperBodyOverrideWeight = 0.0f;
		AimOffsetBlendWeight = 1.0f;
	}
	else
	{
		bool bRecentlyFired = MainAnimInstance->TimeSinceFiredWeapon < RaiseWeaponAfterFiringDuration; // 刚刚开过火
		bool bAbnormalADS = bGameplayTagIsADS && (bIsCrouching || !bIsOnGround);	// 异常状态开镜: 下蹲开枪或者空中开枪
		static const FName HipfireCurveName(TEXT("applyHipfireOverridePose"));	// 优化: 避免每帧实例化
		bool bCurveForceHipFire = GetCurveValue(HipfireCurveName) > 0;	// 动画曲线强制要求
		if (bRecentlyFired || bAbnormalADS || bCurveForceHipFire)
		{
			// 强制使用腰射姿态覆盖
			HipFireUpperBodyOverrideWeight = 1.0f;
			AimOffsetBlendWeight = 1.0f;
		}
		else
		{
			// 智能持枪过度
			HipFireUpperBodyOverrideWeight = FMath::FInterpTo(HipFireUpperBodyOverrideWeight, 0.0f, DeltaSeconds, 1.0f); // 平滑褪去腰射权重
			bool bIsRegularMotion = FMath::Abs(MainAnimInstance->RootYawOffset) < 10.0f && MainAnimInstance->bHasAcceleration;
			float TargetAimFloat = bIsRegularMotion ? HipFireUpperBodyOverrideWeight : 1.0f; // 常规跑动不持枪
			AimOffsetBlendWeight = FMath::FInterpTo(AimOffsetBlendWeight, TargetAimFloat, DeltaSeconds, 10.0f);
		}
	}
}

void UItemAnimLayersBase::UpdateJumpFallData(float DeltaSeconds)
{
	bool bIsFalling = MainAnimInstance->bIsFalling;
	bool bIsJumping = MainAnimInstance->bIsJumping;
	
	if (bIsFalling)
	{
		TimeFalling += DeltaSeconds;
	}
	else
	{
		if (bIsJumping)
		{
			TimeFalling = 0.0f;
		}
	}
}

void UItemAnimLayersBase::UpdateSkelControlData(float DeltaSeconds)
{
	// 获取全局基础权重：禁用则为 0，启用则为 1
	float BaseIKAlpha = bDisableHandIK ? 0.0f : 1.0f;
	
	// 缓存曲线名称避免每帧构造析构
	static const FName DisableRHandIKName(TEXT("DisableRHandIK"));
	static const FName DisableLHandIKName(TEXT("DisableLHandIK"));
	
	// 计算右手 IK 最终权重 (基础值 - 曲线值，并钳制在 0~1)
	float RHandCurveValue = GetCurveValue(DisableRHandIKName);
	HandIKRightAlpha = FMath::Clamp(BaseIKAlpha - RHandCurveValue, 0.0f, 1.0f);

	// 计算左手 IK 最终权重
	float LHandCurveValue = GetCurveValue(DisableLHandIKName);
	HandIKLeftAlpha = FMath::Clamp(BaseIKAlpha - LHandCurveValue, 0.0f, 1.0f);
}
