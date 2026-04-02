#pragma once

#include "CoreMinimal.h"
#include "Engine/UserDefinedEnum.h"
#include "LCSRootYawOffsetMode.generated.h"

UENUM(BlueprintType)
enum class ELCSRootYawOffsetMode : uint8
{
	BlendOut   UMETA(DisplayName = "Blend Out"),	// 角色开始起步跑动，Offset 强制快速平滑归零
	Hold       UMETA(DisplayName = "Hold"),			// 角色正在播转身动画，暂停计算新的鼠标输入
	Accumulate UMETA(DisplayName = "Accumulate")	// 角色站着不动，转鼠标，Offset 开始累加
};

