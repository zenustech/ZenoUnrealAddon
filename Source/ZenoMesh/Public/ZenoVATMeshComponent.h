// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "ZenoVATMeshComponent.generated.h"


UCLASS(ClassGroup=(Zeno), meta=(BlueprintSpawnableComponent))
class ZENOMESH_API UZenoVATMeshComponent : public UMeshComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UZenoVATMeshComponent();

protected:
	/** Position Texture */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT", DisplayName = "Position Texture", meta = (ZenoVat))
	TSoftObjectPtr<UTexture2D> PositionTexturePath;

	/** Should the animation auto started? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT", meta = (ZenoVat))
	bool bAutoPlay = false;

	/** Number of frames */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT", DisplayName = "Number of Frames", meta = (ZenoVat))
	int32 TotalFrame = 0;

	/** Height of texture */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VAT", DisplayName = "Height of Texture", meta = (ZenoVat))
	int32 TextureHeight = 0;

	/** Playing speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT", DisplayName = "Animation Speed", meta = (ZenoVat))
	float PlaySpeed = 1.0f;

	/** Bounds to scale back position data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT", DisplayName = "Maximum Bounds", meta = (ZenoVat))
	FVector3f MaxBounds = FVector3f::Zero();
	
	/** Bounds to scale back position data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT", DisplayName = "Minimum Bounds", meta = (ZenoVat))
	FVector3f MinBounds = FVector3f::Zero();

	/** Bounds to scale back position data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT", DisplayName = "Material")
	UMaterialInterface* MeshMaterial = nullptr;

	/** Current Frame */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT", DisplayName = "Current Frame", meta = (ZenoVat))
	int32 CurrentFrame = 0;
	
public:
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

	virtual void PostInitProperties() override;

	void UpdateVarInfoToRenderThread() const;

protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const override;
#endif // WITH_EDITOR

	friend class FZenoVatMeshSceneProxy;
	
};

inline void InitOrUpdateResource(FRenderResource* Resource)
{
	if (!Resource->IsInitialized())
	{
		Resource->InitResource();
	}
	else
	{
		Resource->UpdateRHI();
	}
}

struct FZenoVatMeshUniformData
{
	FVector3f BoundsMin = FVector3f::Zero();
	FVector3f BoundsMax = FVector3f::Zero();
	int32 TotalFrame = 0;
	int32 TextureHeight = 0;
	float PlaySpeed = 1.0f;
	bool bAutoPlay = true;
	TStrongObjectPtr<UTexture2D> PositionTexture;
	int32 CurrentFrame = 0;
};

struct FZenoVatMeshUniformDataWrapper : public FOneFrameResource
{
	FZenoVatMeshUniformData Data;
};
