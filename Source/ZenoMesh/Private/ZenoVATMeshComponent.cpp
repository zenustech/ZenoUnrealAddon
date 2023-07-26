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
#if 0
	FBoxSphereBounds Result { ForceInitToZero };

	const FVector3d VatBoundExtent = FVector3d(MaxBounds - MinBounds);
	const FBoxSphereBounds SingleVatBounds = FBoxSphereBounds( FVector::ZeroVector, VatBoundExtent, VatBoundExtent.GetMax());

	TArray<USceneComponent*> ChildrenComponents;
	GetChildrenComponents(false, ChildrenComponents);
	for (const USceneComponent* Child : ChildrenComponents)
	{
		if (Child->IsA<UZenoVATInstanceComponent>())
		{
			FTransform ChildTransform = Child->GetRelativeTransform();
			Result = Result + SingleVatBounds.TransformBy(ChildTransform).TransformBy(LocalToWorld);
		}
	}

	if (Result == FBoxSphereBounds(ForceInitToZero))
	{
		Result = SingleVatBounds;
	}
	
	return Result.ExpandBy(BoundsScale);
#else // Disable bounds calculation for debug
    return FBoxSphereBounds(LocalToWorld.GetLocation(), FVector(1000000.0f, 1000000.0f, 1000000.0f), 1000000.0f);
#endif
}

void UZenoVATMeshComponent::PostLoad()
{
	Super::PostLoad();
	UpdateBounds();
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

void UZenoVATMeshComponent::SetCurrentFrame(int32 Value)
{
	CurrentFrame = Value;
	UpdateVarInfoToRenderThread();
}

void UZenoVATMeshComponent::SetCurrentFrame_Interp(float Value)
{
	CurrentFrame_Interp = Value;
	SetCurrentFrame(FMath::Clamp(Value, 0, TotalFrame - 1));
}

void UZenoVATMeshComponent::CreateRenderState_Concurrent(FRegisterComponentContext* Context)
{
	Super::CreateRenderState_Concurrent(Context);
	UpdateInstanceTransformsToRenderThread();
}

#if WITH_EDITOR
void UZenoVATMeshComponent::PostEditComponentMove(bool bFinished)
{
	Super::PostEditComponentMove(bFinished);
	UpdateInstanceTransformsToRenderThread();
}

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
			UpdateBounds();
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
		UpdateBounds();
		UpdateInstanceTransformsToRenderThread();
	}
}

void UZenoVATMeshComponent::OnChildDetached(USceneComponent* ChildComponent)
{
	Super::OnChildDetached(ChildComponent);
	if (ChildComponent->IsA<UZenoVATInstanceComponent>())
	{
		UpdateBounds();
		UpdateInstanceTransformsToRenderThread();
	}
}

void UZenoVATMeshComponent::OnRegister()
{
	Super::OnRegister();
	UpdateBounds();
}

void UZenoVATMeshComponent::PostInterpChange(FProperty* PropertyThatChanged)
{
	static FName CurrentFrameName = GET_MEMBER_NAME_CHECKED(UZenoVATMeshComponent, CurrentFrame_Interp);
	
	Super::PostInterpChange(PropertyThatChanged);

	const FName PropertyName = PropertyThatChanged->GetFName();
	if (PropertyName == CurrentFrameName)
	{
		CurrentFrame = FMath::Clamp(FMath::RoundToInt(CurrentFrame_Interp), 0, TotalFrame - 1);
		UpdateVarInfoToRenderThread();
		UpdateInstanceTransformsToRenderThread();
	}
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

void UZenoVATInstanceComponent::CreateRenderState_Concurrent(FRegisterComponentContext* Context)
{
	Super::CreateRenderState_Concurrent(Context);
	NotifyParentToRebuildData();
}

void UZenoVATInstanceComponent::PostEditComponentMove(bool bFinished)
{
	Super::PostEditComponentMove(bFinished);
	if (bFinished)
	{
		NotifyParentToRebuildData();
	}
}
#endif // WITH_EDITOR
