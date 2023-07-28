// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "ZenoVATMeshComponent.generated.h"


class UZenoMeshInstance;

UCLASS(ClassGroup=(Zeno), meta=(BlueprintSpawnableComponent), DefaultToInstanced)
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

	/** Normal Texture */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT", DisplayName = "Normal Texture", meta = (ZenoVat))
	TSoftObjectPtr<UTexture2D> NormalTexturePath;

	/** Should the animation auto started? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT", meta = (ZenoVat))
	bool bAutoPlay = false;

	/** Number of frames */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT", DisplayName = "Number of Frames", meta = (ZenoVat, UIMin = 0, ClampMin = 0))
	int32 TotalFrame = 0;

	/** Height of texture */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VAT", DisplayName = "Height of Texture", meta = (ZenoVat))
	int32 TextureHeight = 0;

	/** Playing speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT", DisplayName = "Animation Speed", meta = (ZenoVat))
	float PlaySpeed = 24.0f;

	/** Bounds to scale back position data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT", DisplayName = "Maximum Bounds", meta = (ZenoVat))
	FVector3f MaxBounds = FVector3f::Zero();
	
	/** Bounds to scale back position data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT", DisplayName = "Minimum Bounds", meta = (ZenoVat))
	FVector3f MinBounds = FVector3f::Zero();

	/** Material of this mesh, required enable UV channels */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT", DisplayName = "Material")
	UMaterialInterface* MeshMaterial = nullptr;

	/** Current Frame */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT", DisplayName = "Current Frame", meta = (ZenoVat, UIMin = 0, ClampMin = 0))
	int32 CurrentFrame = 0;

	/** Current Frame Interp */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT", DisplayName = "Current Frame(For Interp)", AdvancedDisplay, meta = (ZenoVat, UIMin = 0, ClampMin = 0), Interp)
	float CurrentFrame_Interp = 0;

	/** Base mesh data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT", DisplayName = "Mesh Data")
	UZenoMeshInstance* MeshData = nullptr;

	/** Time passed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VAT", DisplayName = "Time Passed", AdvancedDisplay)
	float TimePassed = 0.0f;

	/** Count of instances */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VAT", DisplayName = "Num Instances", meta = (UIMin = 1, ClampMin = 1, UIMax = 20, ClampMax = 30, SliderExponent = 1.0f))
	mutable int32 NumInstances = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VAT")
	mutable TArray<FMatrix> CachedInstanceTransforms = { FMatrix::Identity };
public:
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

	virtual void PostLoad() override;
	
	virtual void Serialize(FArchive& Ar) override;

	void UpdateVarInfoToRenderThread() const;

	void UpdateInstanceCount() const;

	UFUNCTION(BlueprintCallable)
	void UpdateInstanceTransformsToRenderThread() const;

	UFUNCTION(CallInEditor)
	void SetCurrentFrame(int32 Value);
	
	UFUNCTION(CallInEditor)
	void SetCurrentFrame_Interp(float Value);

	virtual void CreateRenderState_Concurrent(FRegisterComponentContext* Context) override;

protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const override;
	virtual void PostEditComponentMove(bool bFinished) override;
#endif // WITH_EDITOR
	
	virtual void OnChildAttached(USceneComponent* ChildComponent) override;
	virtual void OnChildDetached(USceneComponent* ChildComponent) override;

	virtual void OnRegister() override;
	virtual void PostInterpChange(FProperty* PropertyThatChanged) override;

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
	UTexture2D* PositionTexture = nullptr;
	int32 CurrentFrame = 0;
	UTexture2D* NormalTexture = nullptr;
	TArray<FMatrix> InstancesToWorld;
};

struct FZenoVatMeshUniformDataWrapper : public FOneFrameResource
{
	FZenoVatMeshUniformData Data;
};

/**
 * Component to keep track of the instance data
 * like position, rotation, scale, etc.
 */
UCLASS(ClassGroup=(Zeno), meta=(BlueprintSpawnableComponent))
class ZENOMESH_API UZenoVATInstanceComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	void NotifyParentToRebuildData() const;

protected:
	virtual void OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport) override;
	virtual void CreateRenderState_Concurrent(FRegisterComponentContext* Context) override;
#if WITH_EDITOR
	virtual void PostEditComponentMove(bool bFinished) override;
#endif // WITH_EDITOR
};
