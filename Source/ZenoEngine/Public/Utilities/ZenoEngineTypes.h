// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZenoEngineTypes.generated.h"

UENUM()
enum class EZenoParamType : int8
{
	Invalid = -1,
	Float = 0 UMETA(Default),
	Integer,
	Num UMETA(Hidden),
};
