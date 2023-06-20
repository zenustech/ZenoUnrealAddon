// Fill out your copyright notice in the Description page of Project Settings.


#include "PCG/DataSource/ZenoPCGData.h"

UZenoPCGData::UZenoPCGData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		UID = ++UIDCounter;
	}
}
