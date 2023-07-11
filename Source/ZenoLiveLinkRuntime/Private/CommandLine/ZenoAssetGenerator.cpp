// Fill out your copyright notice in the Description page of Project Settings.


#include "CommandLine/ZenoAssetGenerator.h"

int32 UZenoAssetGeneratorCommandlet::Main(const FString& Params)
{
	UE_LOG(LogTemp, Display, TEXT("ZenoAssetGenerator::Main\nCommand line params: %s"), *Params);
	
	return Super::Main(Params);
}
