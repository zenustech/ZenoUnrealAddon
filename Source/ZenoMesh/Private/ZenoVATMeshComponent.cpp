// Fill out your copyright notice in the Description page of Project Settings.


#include "ZenoVATMeshComponent.h"

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

void UZenoVATMeshComponent::PostInitProperties()
{
	Super::PostInitProperties();
	UpdateBounds();
	MarkRenderTransformDirty();
}

#if WITH_EDITOR
void UZenoVATMeshComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property != nullptr)
	{
		if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(UZenoVATMeshComponent, PositionTexturePath))
		{
			if (const UTexture2D* PositionTexture =  PositionTexturePath.LoadSynchronous(); IsValid(PositionTexture))
			{
				TextureHeight = PositionTexture->GetImportedSize().Y;
			}
		}
	}
}

void UZenoVATMeshComponent::GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const
{
	Super::GetUsedMaterials(OutMaterials, bGetDebugMaterials);
}
#endif // WITH_EDITOR
