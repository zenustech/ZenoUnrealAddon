// Fill out your copyright notice in the Description page of Project Settings.


#include "ZenoVATMeshComponent.h"

#include "ZenoMeshDescriptor.h"
#include "ZenoVatMeshSceneProxy.h"
#include "UObject/SoftObjectPtr.h"


////////////////////////////////////////////////////////////////////////////
/// Implementation of UZenoVATMeshComponent
///////////////////////////////////////////////////////////////////////////
UZenoVATMeshComponent::UZenoVATMeshComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	bAutoRegister	= true;
	bWantsInitializeComponent = true;
	bAutoActivate	= true;
}

FPrimitiveSceneProxy* UZenoVATMeshComponent::CreateSceneProxy()
{
	return new FZenoVatMeshSceneProxy(this);
}

void UZenoVATMeshComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UZenoVATMeshComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

FBoxSphereBounds UZenoVATMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FBoxSphereBounds NewBounds(FBox{ FVector { -1000.0 }, FVector { 1000.0 } }.TransformBy(LocalToWorld));

	NewBounds.BoxExtent *= BoundsScale;
	NewBounds.SphereRadius *= BoundsScale;
	
	return NewBounds;
}

void UZenoVATMeshComponent::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
	if (Ar.IsLoading())
	{
		MarkRenderStateDirty();
	}
}

void UZenoVATMeshComponent::UpdateVarInfoToRenderThread() const
{
	if (SceneProxy)
	{
		const FZenoVatMeshSceneProxy* VatSceneProxy = static_cast<FZenoVatMeshSceneProxy*>(SceneProxy);
		FZenoVatMeshUniformData Data;
		Data.bAutoPlay = bAutoPlay;
		Data.BoundsMax = MaxBounds;
		Data.BoundsMin = MinBounds;
		Data.PlaySpeed = PlaySpeed;
		Data.TotalFrame = TotalFrame;
		Data.TextureHeight = TextureHeight;
		Data.CurrentFrame = CurrentFrame;
		if (PositionTexturePath.IsValid())
		{
			Data.PositionTexture = PositionTexturePath.LoadSynchronous();
		}
		ENQUEUE_RENDER_COMMAND(UpdateZenoVatInfo)(
			[Data, VatSceneProxy](FRHICommandListImmediate& RHICmdList)
			{
				VatSceneProxy->SetVatInfo_RenderThread(Data);
			}
		);
	}
}

#if WITH_EDITOR
void UZenoVATMeshComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.MemberProperty != nullptr)
	{
		const FName MemberPropertyName = PropertyChangedEvent.GetMemberPropertyName();
		if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UZenoVATMeshComponent, PositionTexturePath))
		{
			if (const UTexture2D* PositionTexture =  PositionTexturePath.LoadSynchronous(); IsValid(PositionTexture))
			{
				TextureHeight = PositionTexture->GetImportedSize().Y;
				MarkRenderStateDirty();
			}
		}
		else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UZenoVATMeshComponent, MeshData))
		{
			if (const UZenoMeshInstance* MeshInstance = Cast<UZenoMeshInstance>(MeshData); IsValid(MeshInstance))
			{
				MarkRenderStateDirty();
			}
			else
			{
				MeshData = nullptr;
			}
		}
		else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UZenoVATMeshComponent, MeshMaterial))
		{
			if (SceneProxy)
			{
				ENQUEUE_RENDER_COMMAND(UpdateMaterial) (
					 [this] (FRHICommandListImmediate& RHICmdList)
					 {
					 	if (SceneProxy)
					 	{
					 		FZenoVatMeshSceneProxy* VatSceneProxy = static_cast<FZenoVatMeshSceneProxy*>(SceneProxy);
					 		VatSceneProxy->SetMaterial_RenderThread(MeshMaterial);
					 	}
						  
					 }
				);
			}
		}
		
		if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UZenoVATMeshComponent, CurrentFrame))
		{
			if (CurrentFrame >= TotalFrame)
			{
				CurrentFrame %= FMath::Max(1, TotalFrame);
			}
		}
		if (PropertyChangedEvent.MemberProperty->HasMetaData(TEXT("ZenoVat")))
		{
			UpdateVarInfoToRenderThread();
		}
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UZenoVATMeshComponent::GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const
{
	Super::GetUsedMaterials(OutMaterials, bGetDebugMaterials);
	OutMaterials.Add(MeshMaterial);
}
#endif // WITH_EDITOR
