// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/UserDefinedEnum.h"
#include "LCSCardinalDirection.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class ELCSCardinalDirection : uint8
{
  Forward    UMETA(DisplayName = "Forward"),
  Backward   UMETA(DisplayName = "Backward"),
  Left       UMETA(DisplayName = "Left"),
  Right      UMETA(DisplayName = "Right")
};
