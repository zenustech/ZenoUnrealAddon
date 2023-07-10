﻿// Fill out your copyright notice in the Description page of Project Settings.


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
	PrimaryComponentTick.TickGroup = TG_DuringPhysics;
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

	if (!bAutoPlay)
	{
		return;
	}
	TimePassed += DeltaTime;
	const int32 NewFrameNum = static_cast<int32>(FMath::Fmod(TimePassed * PlaySpeed, TotalFrame));
	if (CurrentFrame != NewFrameNum)
	{
		CurrentFrame = NewFrameNum;
		UpdateVarInfoToRenderThread();
	}
}

FBoxSphereBounds UZenoVATMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FBoxSphereBounds NewBounds(FBox{ FVector { MinBounds }, FVector { MaxBounds } }.TransformBy(LocalToWorld));
	
	NewBounds.BoxExtent *= BoundsScale;
	NewBounds.SphereRadius *= BoundsScale;
	
	return NewBounds;
}

void UZenoVATMeshComponent::PostLoad()
{
	Super::PostLoad();
#if WITH_EDITOR
	UpdateBounds();
#endif // WITH_EDITOR
	UpdateInstanceTransformsToRenderThread();
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
		if (!PositionTexturePath.IsNull())
		{
			Data.PositionTexture = PositionTexturePath.LoadSynchronous();
		}
		if (!NormalTexturePath.IsNull())
		{
			Data.NormalTexture = NormalTexturePath.LoadSynchronous();
		}
		Data.InstancesToWorld = CachedInstanceTransforms;
		ENQUEUE_RENDER_COMMAND(UpdateZenoVatInfo)(
			[Data, VatSceneProxy](FRHICommandListImmediate& RHICmdList)
			{
				VatSceneProxy->SetVatInfo_RenderThread(Data);
			}
		);
	}
}

void UZenoVATMeshComponent::UpdateInstanceCount() const
{
	TArray<USceneComponent*> ChildrenComponents;
	GetChildrenComponents(false, ChildrenComponents);

	int32 Result = 0;
	for (USceneComponent* ChildComponent : ChildrenComponents)
	{
		if (ChildComponent->IsA<UZenoVATInstanceComponent>())
		{
			Result ++;
		}
	}

	NumInstances = Result;
}

void UZenoVATMeshComponent::UpdateInstanceTransformsToRenderThread() const
{
	if (!SceneProxy)
	{
		return;
	}
	
	TArray<USceneComponent*> ChildrenComponents;
	GetChildrenComponents(false, ChildrenComponents);

	UpdateInstanceCount();

	TArray<FMatrix> InstanceTransforms;
	InstanceTransforms.Reserve(NumInstances);
	for (const USceneComponent* ChildComponent : ChildrenComponents)
	{
		if (ChildComponent->IsA<UZenoVATInstanceComponent>())
		{
			InstanceTransforms.Add(ChildComponent->GetRelativeTransform().ToMatrixWithScale());
		}
	}

	if (InstanceTransforms.IsEmpty())
	{
		InstanceTransforms.Add(FMatrix::Identity);
	}

	CachedInstanceTransforms = InstanceTransforms;

	ENQUEUE_RENDER_COMMAND(UpdateZenoVatInstanceTransform)([InstanceTransforms, SceneProxy = static_cast<FZenoVatMeshSceneProxy*>(SceneProxy)] (FRHICommandListImmediate& RHICmdList)
	{
		SceneProxy->SetInstanceTransforms_RenderThread(InstanceTransforms);
	});
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
				MinBounds = MeshInstance->VatMeshData.BoundsMin;
				MaxBounds = MeshInstance->VatMeshData.BoundsMax;
				TotalFrame = MeshInstance->VatMeshData.FrameNum;
				if (MeshInstance->VatMeshData.FrameNum == 0)
				{
					TotalFrame = TextureHeight / FMath::Max(1, MeshInstance->VatMeshData.RowsPerFrame);
				}
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
#if WITH_EDITOR
			UpdateBounds();
#endif // WITH_EDITOR
		}
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UZenoVATMeshComponent::GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const
{
	Super::GetUsedMaterials(OutMaterials, bGetDebugMaterials);
	OutMaterials.Add(MeshMaterial);
}

void UZenoVATMeshComponent::OnChildAttached(USceneComponent* ChildComponent)
{
	Super::OnChildAttached(ChildComponent);
	if (ChildComponent->IsA<UZenoVATInstanceComponent>())
	{
		UpdateInstanceTransformsToRenderThread();
	}
}

void UZenoVATMeshComponent::OnChildDetached(USceneComponent* ChildComponent)
{
	Super::OnChildDetached(ChildComponent);
	if (ChildComponent->IsA<UZenoVATInstanceComponent>())
	{
		UpdateInstanceTransformsToRenderThread();
	}
}

void UZenoVATMeshComponent::OnRegister()
{
	Super::OnRegister();
	UpdateInstanceTransformsToRenderThread();
}

void UZenoVATInstanceComponent::NotifyParentToRebuildData() const
{
	if (const UZenoVATMeshComponent* MeshComponent = Cast<UZenoVATMeshComponent>(GetAttachParent()))
	{
		MeshComponent->UpdateInstanceTransformsToRenderThread();
	}
}

void UZenoVATInstanceComponent::OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	Super::OnUpdateTransform(UpdateTransformFlags, Teleport);
	NotifyParentToRebuildData();
}

void UZenoVATInstanceComponent::OnComponentCreated()
{
	Super::OnComponentCreated();
	NotifyParentToRebuildData();
}

void UZenoVATInstanceComponent::OnRegister()
{
	Super::OnRegister();
	NotifyParentToRebuildData();
}
#endif // WITH_EDITOR
