// Fill out your copyright notice in the Description page of Project Settings.


#include "ZenoBasicMeshComponent.h"

#include "DynamicMeshBuilder.h"

/** Represents a UZenoBasicMeshComponent to the scene manager. */
class FZenoBasicSceneProxy final : public FPrimitiveSceneProxy
{
public:
	virtual SIZE_T GetTypeHash() const override
	{
		static SIZE_T UniquePointer;
		return reinterpret_cast<SIZE_T>(&UniquePointer);
	}

	FZenoBasicSceneProxy(const UZenoBasicMeshComponent* Component)
		: FPrimitiveSceneProxy(Component)
		, VertexFactory(GetScene().GetFeatureLevel(), "FZenoBasicSceneProxy")
	{
		bWillEverBeLit = true;

		// VertexBuffers.InitFromDynamicVertex(&VertexFactory, );

		BeginInitResource(&IndexBuffer);
	}

	virtual ~FZenoBasicSceneProxy() override
	{
		IndexBuffer.ReleaseResource();
		VertexFactory.ReleaseResource();
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_ZenoBasicSceneProxy_GetDynamicMeshElements);
	}

	virtual uint32 GetMemoryFootprint() const override
	{
		return sizeof(*this) + GetAllocatedSize();
	}

	virtual void OnTransformChanged() override
	{
		Origin = GetLocalToWorld().GetOrigin();
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = IsShown(View);
		Result.bDynamicRelevance = true;
		Result.bShadowRelevance = IsShadowCast(View);
		Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
		Result.bVelocityRelevance = DrawsVelocity() && Result.bOpaque && Result.bRenderInMainPass;

		return Result;
	}

private:
	static TArray<FDynamicMeshVertex> BuildDynamicMeshVertices(const FRawMesh& RawMesh)
	{
		return {};
	}

private:
	FStaticMeshVertexBuffers VertexBuffers;
	FDynamicMeshIndexBuffer32 IndexBuffer;
	FLocalVertexFactory VertexFactory;

	FVector Origin;
};

// Mesh Component
UZenoBasicMeshComponent::UZenoBasicMeshComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UZenoBasicMeshComponent::BeginPlay()
{
	Super::BeginPlay();

}

FPrimitiveSceneProxy* UZenoBasicMeshComponent::CreateSceneProxy()
{
	return new FZenoBasicSceneProxy(this);
}


// Called every frame
void UZenoBasicMeshComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                            FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

