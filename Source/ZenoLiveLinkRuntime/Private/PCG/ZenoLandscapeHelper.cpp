#include "PCG/ZenoLandscapeHelper.h"

#if WITH_EDITOR
#include "Editor.h"
#endif // WITH_EDITOR

#include "Landscape.h"
#include "LandscapeDataAccess.h"
#include "LandscapeEdit.h"
#include "LandscapeProxy.h"
#include "Engine/SplineMeshActor.h"
#include "Engine/StaticMeshActor.h"
#include "PCG/ZenoPCGSubsystem.h"
#include "PCG/Grid/ZenoPCGLandscapeCache.h"
#include "PCG/DataSource/ZenoPCGLandscapeData.h"
#include "UObject/ConstructorHelpers.h"

bool Zeno::Helper::IsRuntimeOrPIE()
{
#if WITH_EDITOR
	return (GEditor && GEditor->PlayWorld) || GIsPlayInEditorWorld || IsRunningGame();
#else
	return true;
#endif // WITH_EDITOR
}

FBox Zeno::Helper::GetGridBounds(const AActor* Actor, const UZenoPCGVolumeComponent* Component)
{
	FBox Bounds(ForceInit);

	if (const ALandscapeProxy* LandscapeActor = Cast<const ALandscape>(Actor))
	{
		Bounds = GetLandscapeBounds(LandscapeActor);
	} else if (IsValid(Actor))
	{
		Bounds = GetActorBounds(Actor);
	}
	
	return Bounds;
}

FBox Zeno::Helper::GetActorBounds(const AActor* InActor, bool bIgnorePCGCreatedComponent/* = true */)
{
	FBox Box(ForceInit);
	const bool bNonColliding = true;
	const bool bIncludeFromChildActors = true;

	if (IsValid(InActor))
	{
		InActor->ForEachComponent<UPrimitiveComponent>(bIncludeFromChildActors, [bNonColliding, bIgnorePCGCreatedComponent, &Box](const UPrimitiveComponent* InComponent)
		{
			if ((bNonColliding || InComponent->IsCollisionEnabled()) && (!bIgnorePCGCreatedComponent || !InComponent->ComponentTags.Contains(DefaultPCGTag)))
			{
				Box += InComponent->Bounds.GetBox();
			}
		});
	}

	return Box;
}

FBox Zeno::Helper::GetLandscapeBounds(const ALandscapeProxy* InLandscapeProxy)
{
	check(InLandscapeProxy);

	if (const ALandscape* Landscape = Cast<const ALandscape>(InLandscapeProxy); Landscape && Landscape->GetLandscapeGuid().IsValid())
	{
		if (Landscape->GetLandscapeInfo() == nullptr)
		{
			return FBox(ForceInit);
		}
#if WITH_EDITOR
		if (!IsRuntimeOrPIE())
		{
			return Landscape->GetCompleteBounds();
		}
		else
#endif // WITH_EDITOR
		{
			return Landscape->GetLoadedBounds();
		}
	}
	else
	{
		return GetActorBounds(InLandscapeProxy);
	}
}

TArray<TWeakObjectPtr<ALandscapeProxy>> Zeno::Helper::GetLandscapeProxies(const UWorld* InWorld, const FBox& InBounds)
{
	TArray<TWeakObjectPtr<ALandscapeProxy>> LandscapeProxies;

	if (InBounds.IsValid)
	{
		for (TObjectIterator<ALandscapeProxy> It; It; ++It)
		{
			if (It->GetWorld() == InWorld)
			{
				const FBox LandscapeBounds = GetLandscapeBounds(*It);
				if (LandscapeBounds.IsValid && LandscapeBounds.Intersect(InBounds))
				{
					LandscapeProxies.Add(*It);
				}
			}
		}
	}
	
	return LandscapeProxies;
}

TArray<uint16> Zeno::Helper::GetHeightDataInBound(const ALandscapeProxy* Landscape, FBox& InOutBound, FIntPoint& OutSize, bool& OutSuccess)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(Zeno::Helper::GetHeightDataInBound);
	
	// TODO [darc] : Supporting multiple landscapes :
	check(IsValid(Landscape) && InOutBound.IsValid);

	const FBox ActorBound = InOutBound.InverseTransformBy(Landscape->LandscapeActorToWorld());
	const FBox LandscapeBound = Landscape->GetComponentsBoundingBox().InverseTransformBy(Landscape->LandscapeActorToWorld());
	
	int32 MinX = FMath::FloorToInt((ActorBound.Min.X));
	int32 MinY = FMath::FloorToInt((ActorBound.Min.Y));
	int32 MaxX = FMath::CeilToInt((ActorBound.Max.X));
	int32 MaxY = FMath::CeilToInt((ActorBound.Max.Y));

	MinX = FMath::Clamp(MinX, 0, LandscapeBound.Min.X);
	MinY = FMath::Clamp(MinY, 0, LandscapeBound.Min.Y);
	MaxX = FMath::Clamp(MaxX, 0, LandscapeBound.Max.X);
	MaxY = FMath::Clamp(MaxY, 0, LandscapeBound.Max.Y);

	const size_t VertsX = MaxX - MinX + 1, VertsY = MaxY - MinY + 1;
	
	TArray<uint16> HeightData;
	HeightData.SetNumZeroed(VertsX * VertsY);
	
	FLandscapeEditDataInterface EditDataInterface(Landscape->GetLandscapeInfo());
	EditDataInterface.GetHeightData(MinX, MinY, MaxX, MaxY, HeightData.GetData(), 0);

	if (MinX > MaxX || MinY > MaxY)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Landscape data found."));
		OutSuccess = false;
		return {};
	}

	OutSize.X = VertsX;
	OutSize.Y = VertsY;
	OutSuccess = true;
	InOutBound.Min.X = MinX;
	InOutBound.Min.Y = MinY;
	InOutBound.Max.X = MaxX;
	InOutBound.Max.Y = MaxY;
	InOutBound = InOutBound.TransformBy(Landscape->LandscapeActorToWorld());

	return HeightData;
}

TArray<FVector> Zeno::Helper::ScatterPoints(ALandscapeProxy* Landscape, const uint32 NumPoints, const int32 Seed, const FBox& InBound)
{
	UZenoPCGLandscapeData* Data = NewObject<UZenoPCGLandscapeData>();
	Data->Initialize({ Landscape }, GetLandscapeBounds(Landscape), true, false);
	TArray<FVector> Points;

	if (!IsValid(Landscape))
	{
		return Points;
	}

	Points.Reserve(NumPoints);
	const FRandomStream RandomStream { Seed };

	for (uint32 PointIdx = 0; PointIdx < NumPoints; ++PointIdx)
	{
		FVector RandomPoint = RandomStream.RandPointInBox(InBound);
		FZenoPCGPoint OutPoint;
		Data->SamplePoint(FTransform {RandomPoint}, InBound, OutPoint, nullptr);
		const FVector Point = OutPoint.Transform.GetLocation();
		Points.Add(Point);
	}
	
	return Points;
}

TArray<uint16> Zeno::Helper::GetHeightDataInBound(ALandscapeProxy* Landscape, FBox& InOutBound, FIntPoint& OutSize)
{
	TArray<uint16> HeightData;
	if (!IsValid(Landscape))
	{
		return {};
	}

#if 0
	UZenoPCGLandscapeData* Data = NewObject<UZenoPCGLandscapeData>();
	Data->Initialize({Landscape}, GetLandscapeBounds(Landscape), true, false);

	const ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();
	const int32 ComponentSizeQuads = LandscapeInfo->ComponentSizeQuads;
	const int32 SubsectionSizeQuads = LandscapeInfo->SubsectionSizeQuads;

	FIntRect LandscapeExtent;
	LandscapeInfo->GetLandscapeExtent(LandscapeExtent);

	const double SizePerSectionX = LandscapeExtent.Width() / static_cast<double>(ComponentSizeQuads) * SubsectionSizeQuads;
	const double SizePerSectionY = LandscapeExtent.Height() / static_cast<double>(ComponentSizeQuads) * SubsectionSizeQuads;

	const FTransform& LandscapeTransform = Landscape->GetTransform();

	OutSize = 0;
	uint32 MaxY = 0;
	for (double X = InOutBound.Min.X; X < InOutBound.Max.X; X += SizePerSectionX)
	{
		OutSize.Y ++;
		MaxY = 0;
		for (double Y = InOutBound.Min.Y; Y < InOutBound.Max.Y; Y += SizePerSectionY)
		{
			FZenoPCGPoint Point;
			if (Data->ProjectPoint(FTransform { FVector {X, Y, 0.f} }, InOutBound, {}, Point, nullptr))
			{
				FSoftObjectPath CubeMesh { "/Engine/BasicShapes/Cube.Cube" };
				// if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(CubeMesh.TryLoad()); IsValid(StaticMesh))
				// {
				// 	const AStaticMeshActor* Actor = Cast<AStaticMeshActor>(World->SpawnActor(AStaticMeshActor::StaticClass(), &Point.Transform));
				// 	Actor->GetStaticMeshComponent()->SetStaticMesh(StaticMesh);
				// }
				HeightData.Add( LandscapeDataAccess::GetTexHeight(LandscapeTransform.InverseTransformVector(Point.Transform.GetLocation()).Z));
			} else
			{
				UE_LOG(LogTemp, Warning, TEXT("%lf %lf"), X, Y);
				HeightData.Add(0x8000);
			}
			MaxY ++;
		}
	}
	OutSize.X = MaxY;
#else
	ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();
	check(IsValid(LandscapeInfo));	

	FIntRect LandscapeExtent;
	if (!LandscapeInfo->GetLandscapeExtent(LandscapeExtent))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get Landscape Extent. Aborted."));
		return {};
	}


	InOutBound = InOutBound.InverseTransformBy(Landscape->GetTransform());	
	const FVector& BoxMin = InOutBound.Min;
	const FVector& BoxMax = InOutBound.Max;
	const FIntPoint MinInt(FMath::FloorToInt(BoxMin.X), FMath::FloorToInt(BoxMin.Y));
	const FIntPoint MaxInt(FMath::FloorToInt(BoxMax.X), FMath::FloorToInt(BoxMax.Y));
	FIntRect InBoundsRect { MinInt, MaxInt };

	if (!InBoundsRect.Intersect(LandscapeExtent))
	{
		UE_LOG(LogTemp, Error, TEXT("InBounds isn't intersected with Landscape Extent. Aborted."));
		return {};
	}

	auto GetIntersectRect = [] (const FIntRect& Rect1, const FIntRect& Rect2)
	{
		const int32 MaxLeft = FMath::Max(Rect1.Min.X, Rect2.Min.X);
		const int32 MaxTop = FMath::Max(Rect1.Min.Y, Rect2.Min.Y);
		const int32 MinRight = FMath::Min(Rect1.Max.X, Rect2.Max.X);
		const int32 MinBottom = FMath::Min(Rect1.Max.Y, Rect2.Max.Y);
		if (MaxLeft <= MinRight && MaxTop <= MinBottom)
        {
            return FIntRect(MaxLeft, MaxTop, MinRight, MinBottom);
        }
		return FIntRect(0, 0, 0, 0);
	};

	FIntRect IntersectRect = GetIntersectRect(InBoundsRect, LandscapeExtent);
	const int32 DataWidth = IntersectRect.Width() + 1;
	const int32 DataHeight = IntersectRect.Height() + 1;

	HeightData.AddZeroed(DataWidth * DataHeight);

	FLandscapeEditDataInterface LandscapeEdit(LandscapeInfo);
	LandscapeEdit.GetHeightData(IntersectRect.Min.X, IntersectRect.Min.Y, IntersectRect.Max.X, IntersectRect.Max.Y, HeightData.GetData(), 0);

	OutSize.X = DataWidth;
	OutSize.Y = DataHeight;
#endif

	return HeightData;
}
