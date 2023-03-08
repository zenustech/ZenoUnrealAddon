// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ZenoLandscapeEditorObject.generated.h"

UCLASS()
class ZENOLIVELINK_API UZenoLandscapeEditorObject : public UObject
{
	GENERATED_BODY()

	TOptional<FLiveLinkSubjectKey> SelectedSubjectKey;

	friend class UZenoLandscapeTool;
};
