// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "ZenoBasicMeshComponent.generated.h"

UCLASS(ClassGroup=(Zeno), meta=(BlueprintSpawnableComponent))
class ZENOENGINE_API UZenoBasicMeshComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UZenoBasicMeshComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	//~ Begin UPrimitiveComponent Interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	//~ End UPrimitiveComponent Interface.

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

protected:
	TSharedPtr<FRawMesh> MeshData;
};
