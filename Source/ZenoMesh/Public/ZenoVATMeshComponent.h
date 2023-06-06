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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT", DisplayName = "Position Texture")
	TSoftObjectPtr<UTexture2D> PositionTexturePath;

	/** Should the animation auto started? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT")
	bool bAutoPlay = false;

	/** Number of frames */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT", DisplayName = "Number of Frames")
	int32 TotalFrame = 0;

	/** Height of texture */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VAT", DisplayName = "Height of Texture")
	int32 TextureHeight = 0;

	/** Playing speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT", DisplayName = "Animation Speed")
	float PlaySpeed = 1.0f;

	/** Bounds to scale back position data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT", DisplayName = "Maximum Bounds")
	FVector3f MaxBounds = FVector3f::Zero();
	
	/** Bounds to scale back position data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VAT", DisplayName = "Minimum Bounds")
	FVector3f MinBounds = FVector3f::Zero();
	
public:
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
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
