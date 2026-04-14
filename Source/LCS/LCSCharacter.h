// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "LCSCharacter.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS()
class LCS_API ALCSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ALCSCharacter();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	void RegisterMappingContext() const;

	// 输入回调
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void OnJump(const FInputActionValue& Value);
	void OnCrouchStart(const FInputActionValue& Value);
	void OnCrouchEnd(const FInputActionValue& Value);
	
	// 装备武器
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipWeapon();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	class USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	class UCameraComponent* CameraComponent;
	
	// 增强输入
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* CrouchAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JogAction;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;
};
