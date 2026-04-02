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
		Character = OwningCharacter;
		MovementComponent = OwningCharacter->GetCharacterMovement();
	}
}

void ULCSAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
	
	if (!Character.IsValid() || !MovementComponent.IsValid()) return;

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

void ULCSAnimInstance::UpdateLocationData(float DeltaSeconds)
{
	// 距离匹配计算: 离地高度，目标距离等
}

void ULCSAnimInstance::UpdateRotationData(float DeltaSeconds)
{
	
}

void ULCSAnimInstance::UpdateVelocityData(float DeltaSeconds)
{
	
}

void ULCSAnimInstance::UpdateAccelerationData(float DeltaSeconds)
{
	
}

void ULCSAnimInstance::UpdateWallDetection(float DeltaSeconds)
{
	
}

void ULCSAnimInstance::UpdateCharacterStateData(float DeltaSeconds)
{
	
}

void ULCSAnimInstance::UpdateBlendWeightData(float DeltaSeconds)
{
	
}

void ULCSAnimInstance::UpdateRootYawOffset(float DeltaSeconds)
{
	
}

void ULCSAnimInstance::UpdateAimingData(float DeltaSeconds)
{
	
}

void ULCSAnimInstance::UpdateJumpFallData(float DeltaSeconds)
{
	
}
