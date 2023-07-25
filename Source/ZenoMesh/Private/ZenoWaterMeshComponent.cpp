#include "ZenoWaterMeshComponent.h"

#ifdef UE_5_2_OR_LATER
#include "DataDrivenShaderPlatformInfo.h"
#include "MaterialDomain.h"
#include "Materials/MaterialRenderProxy.h"
#endif
#include "MeshDrawShaderBindings.h"
#include "MeshMaterialShader.h"
#include "ZenoMeshBuffer.h"
#include "ZenoMeshCommon.h"
#include "ZenoRiverActor.h"
#include "ZenoSplineComponent.h"
#include "Misc/ScopedSlowTask.h"
#include "UObject/GCObjectScopeGuard.h"

#define LOCTEXT_NAMESPACE "ZenoMesh"

class AZenoRiverActor;

void BeginInitResourceIfNotNull(FRenderResource* Resource)
{
	if (Resource != nullptr) BeginInitResource(Resource);
}

class FZenoWaterMeshSceneProxy : public FPrimitiveSceneProxy {};

FZenoWaterMeshRenderData::FZenoWaterMeshRenderData(bool bInKeepCPUData/* = true*/)
	: bKeepCPUData(bInKeepCPUData)
{
	BufferAllocator = new FZenoMeshBufferAllocator;
	VertexBuffer = new FZenoMeshVertexBuffer(1, 0, true, *BufferAllocator);
	IndexBuffer = new FZenoMeshIndexBuffer(*BufferAllocator);
}

FZenoWaterMeshRenderData::~FZenoWaterMeshRenderData()
{
	delete IndexBuffer;
	delete VertexBuffer;
	delete BufferAllocator;
}

void FZenoWaterMeshRenderData::UploadData_AnyThread() const
{
	auto UploadOperator = [this]()
	{
		check(VertexBuffer != nullptr);
		check(IndexBuffer != nullptr);
		BeginInitResource(VertexBuffer);
		BeginInitResource(IndexBuffer);
		BeginInitResourceIfNotNull(MaterialProxy);

		// Release cpu data if needed
		if (!bKeepCPUData)
		{
			VertexBuffer->Vertices.Empty();
			IndexBuffer->Indices.Empty();
		}
	};
	
	if (IsInRenderingThread())
	{
		UploadOperator();
	}
	else
	{
		ENQUEUE_RENDER_COMMAND(FZenoWaterMeshRenderData_UploadData)([UploadOperator] (FRHICommandListImmediate&)
		{
			UploadOperator();
		});
	}
}

UZenoWaterMeshComponent::UZenoWaterMeshComponent(const FObjectInitializer& Initializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_DuringPhysics;
	bAutoRegister = true;
	bWantsInitializeComponent = true;
	bAutoActivate = true;
}

UZenoWaterMeshComponent::~UZenoWaterMeshComponent()
{
}

void UZenoWaterMeshComponent::BuildRiverMesh(const FZenoRiverBuildInfo& InBuildInfo)
{
	if (!InBuildInfo.Spline.IsValid())
	{
		UE_LOG(LogZenoMesh, Error, TEXT("Invalid spline component, aborted building river mesh."));
		return;
	}
	RenderData = MakeShared<FZenoWaterMeshRenderData>(bKeepBufferInCPU);

	UZenoSplineComponent* SplineComponent = InBuildInfo.Spline.Get();
	// Prevent spline from GC
	FGCObjectScopeGuard SplineGuard(SplineComponent);

	const int32 NumPoints = SplineComponent->GetNumberOfSplinePoints() * Precision;
	const float SplineLength = SplineComponent->GetSplineLength();
	const float Delta = SplineLength / static_cast<float>(NumPoints);
	const int32 NumWidthPoints = FMath::Max(1, InBuildInfo.RiverWidth) * static_cast<float>(Precision) + 1;
	const float WidthDelta = InBuildInfo.RiverWidth / static_cast<float>(NumWidthPoints);
	
	FScopedSlowTask SlowTask(NumPoints, LOCTEXT("BuildRiverMesh", "Building River Mesh"));
	SlowTask.MakeDialogDelayed(3.0f);

	RenderData->VertexBuffer->Vertices.Empty();
	RenderData->VertexBuffer->Vertices.Reserve(NumPoints * NumWidthPoints);
	for (int32 i = 0; i < NumPoints; i++)
	{
		const float Distance = Delta * static_cast<float>(i);
		const FVector CenterPosition = SplineComponent->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::Local);
		const FVector CenterTangent = SplineComponent->GetTangentAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::Local);
		for (int32 j = 0; j < NumWidthPoints; j++)
		{
			const FVector2D TexCoord = FVector2D(static_cast<float>(j) * WidthDelta, Distance / SplineLength);
			const FVector CurrentPosition = CenterPosition + CenterTangent * FVector(static_cast<float>(j) * WidthDelta - InBuildInfo.RiverWidth * 0.5f);
			const FVector CurrentTangent = CenterTangent;
			const FVector CurrentNormal = FVector::CrossProduct(CurrentTangent, FVector::UpVector).GetSafeNormal();
			FZenoMeshVertex Vertex;
			Vertex.Normal = CurrentNormal;
			Vertex.Position = FVector3f(CurrentPosition);
			Vertex.Tangent = CurrentTangent;
			Vertex.TextureCoordinate[0] = FVector2f(TexCoord);
			Vertex.Color = FColor::White; // TODO [darc] : pass other data by color channel :
			RenderData->VertexBuffer->Vertices.Add(Vertex);
			SlowTask.CompletedWork = i * NumWidthPoints + j;
		}
	}

	FIntPoint NumGirds(NumPoints - 1, NumWidthPoints - 1);
	RenderData->IndexBuffer->Indices.Empty();
	RenderData->IndexBuffer->Indices.Reserve(NumGirds.X * NumGirds.Y * 6);
	for (int32 i = 0; i < NumGirds.X; i++)
	{
		for (int32 j = 0; j < NumGirds.Y; j++)
		{
			const int32 Index = i * NumWidthPoints + j;
			RenderData->IndexBuffer->Indices.Add(Index);
			RenderData->IndexBuffer->Indices.Add(Index + NumWidthPoints);
			RenderData->IndexBuffer->Indices.Add(Index + NumWidthPoints + 1);
			RenderData->IndexBuffer->Indices.Add(Index);
			RenderData->IndexBuffer->Indices.Add(Index + NumWidthPoints + 1);
			RenderData->IndexBuffer->Indices.Add(Index + 1);
		}
	}
	
	if (IsValid(WaterMaterial))
	{
		RenderData->MaterialProxy = WaterMaterial->GetRenderProxy();
	}

	// call this in scene proxy
	// RenderData->UploadData_AnyThread();
}

FPrimitiveSceneProxy* UZenoWaterMeshComponent::CreateSceneProxy()
{
	if (RenderData.IsValid())
	{
		return Super::CreateSceneProxy();
	}
	return nullptr;
}

void UZenoWaterMeshComponent::GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const
{
	Super::GetUsedMaterials(OutMaterials, bGetDebugMaterials);
	OutMaterials.Add(WaterMaterial);
}

void UZenoWaterMeshComponent::CreateRenderState_Concurrent(FRegisterComponentContext* Context)
{
	Super::CreateRenderState_Concurrent(Context);
}

FBoxSphereBounds UZenoWaterMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
{
#if 1
	if (AZenoRiverActor* RiverActor = Cast<AZenoRiverActor>(GetAttachParentActor()); IsValid(RiverActor))
	{
		const float LengthMax = FMath::Max(RiverActor->SplineComponent->GetSplineLength(), static_cast<float>(RiverActor->RiverWidth));
		FVector Extent(LengthMax, LengthMax, 10.f );
		Extent *= FVector(BoundsScale);
		return FBoxSphereBounds(FVector::Zero(), Extent, Extent.GetMax()).TransformBy(LocalToWorld);
	}
	return FBoxSphereBounds(LocalToWorld.GetLocation(), FVector(100.0f, 100.0f, 10.0f), 100.0f);
#else // For debug
    return FBoxSphereBounds(LocalToWorld.GetLocation(), FVector(1000000.0f, 1000000.0f, 1000000.0f), 1000000.0f);
#endif
}

#undef LOCTEXT_NAMESPACE
