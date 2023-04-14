﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZenoGraphActor.generated.h"

class UZenoGraphAsset;

UCLASS()
class ZENOENGINE_API AZenoGraphBaseActor : public AActor
{
	GENERATED_BODY()

public:
	AZenoGraphBaseActor(const FObjectInitializer& ObjectInitializer);
	
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere, Category = "Zeno | Graph", DisplayName = "Graph Data")
	UZenoGraphAsset* ZenoGraphAsset;

	friend class UZenoGraphAssetActorFactory;
};

UCLASS()
class ZENOENGINE_API AZenoGraphMeshActor : public AZenoGraphBaseActor
{
	GENERATED_BODY()

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void SetMeshComponent(UStaticMeshComponent* InStaticMeshComponent);
#endif // WITH_EDITOR

public:
	AZenoGraphMeshActor(const FObjectInitializer& ObjectInitializer);


protected:
	UPROPERTY(VisibleAnywhere, Category = "Zeno | Debug", DisplayName = "Mesh Data")
	UStaticMesh* StaticMesh;

	UPROPERTY(VisibleAnywhere, Category = "Zeno | Debug", DisplayName = "Mesh Component")
	UStaticMeshComponent* StaticMeshComponent;
	
	friend class UZenoGraphAssetActorFactory;
	friend class FZenoGraphMeshActorDetailCustomization;
};
