// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZenoGraphActor.generated.h"

class UZenoInputParameter;
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
	UPROPERTY(EditAnywhere, Category = Zeno, DisplayName = "Graph Data", AdvancedDisplay)
	UZenoGraphAsset* ZenoGraphAsset;

	friend class UZenoGraphAssetActorFactory;
	friend class FZenoGraphMeshActorDetailCustomization;
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
	UPROPERTY(VisibleAnywhere, Category = Zeno, DisplayName = "Mesh Data", AdvancedDisplay)
	UStaticMesh* StaticMesh;

	UPROPERTY(VisibleAnywhere, Category = Zeno, DisplayName = "Mesh Component", AdvancedDisplay)
	UStaticMeshComponent* StaticMeshComponent;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = Zeno, AdvancedDisplay)
	FMeshNaniteSettings NaniteSettings;

	UPROPERTY(EditAnywhere, Category = Zeno, AdvancedDisplay)
	FMeshBuildSettings BuildSettings;
	
	UPROPERTY(EditAnywhere, Category = Zeno, AdvancedDisplay)
	FMeshReductionSettings ReductionSettings;

	UPROPERTY(EditAnywhere, Category = Zeno, DisplayName = "Material")
	UMaterialInterface* MeshMaterial;
	
	UPROPERTY(EditAnywhere, Category = Zeno, DisplayName = "Input Data", EditFixedSize)
	TArray<UZenoInputParameter*> InputParameters;
#endif // WITH_EDITORONLY_DATA

	friend class UZenoGraphAssetActorFactory;
	friend class FZenoGraphMeshActorDetailCustomization;
};
