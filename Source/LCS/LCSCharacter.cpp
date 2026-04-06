// Fill out your copyright notice in the Description page of Project Settings.


#include "LCSCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"

// Sets default values
ALCSCharacter::ALCSCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 弹簧臂
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);
	
	// 摄像机
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
}

// Called when the game starts or when spawned
void ALCSCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// 弹簧臂
	SpringArmComponent->bUsePawnControlRotation = true;	// 弹簧臂跟随输入旋转
	SpringArmComponent->TargetArmLength = 300.0f;
	SpringArmComponent->bEnableCameraLag = true; // 开启摄像机位置延迟，人物急停时有滞后感
	SpringArmComponent->CameraLagSpeed = 10.f; // 越小越滞后，越大越及时
	
	// 摄像机
	CameraComponent->bUsePawnControlRotation = false;	// 本身已经挂载在弹簧臂上了，跟随弹簧臂旋转
	
	// 角色朝向控制
	bUseControllerRotationRoll = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	
	// 配置移动组件
	GetCharacterMovement()->bOrientRotationToMovement = false; // 角色朝向移动方向
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;	// 手柄摇杆输入角色开始行走的速度最小阈值
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f; // 急停减速度

	RegisterMappingContext();
}

void ALCSCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ALCSCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookVector = Value.Get<FVector2D>();
	
	if (Controller)
	{
		AddControllerYawInput(LookVector.X);
		AddControllerPitchInput(LookVector.Y);
	}
}

void ALCSCharacter::RegisterMappingContext() const
{
	if (APlayerController *PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

// Called every frame
void ALCSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ALCSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALCSCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALCSCharacter::Look);
	}
}

