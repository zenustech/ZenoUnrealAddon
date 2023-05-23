#include "PCG/ZenoLandscapeHelper.h"

#if WITH_EDITOR
#include "Editor.h"
#endif // WITH_EDITOR

#include "Landscape.h"
#include "LandscapeDataAccess.h"
#include "LandscapeEdit.h"
#include "LandscapeProxy.h"

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

TArray<FVector> Zeno::Helper::ScatterPoints(const ALandscapeProxy* Landscape, const uint32 NumPoints, const int32 Seed, const FBox& InBound)
{
	TArray<FVector> Points;

	if (!IsValid(Landscape))
	{
		return Points;
	}

	const ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();
	if (!IsValid(LandscapeInfo))
	{
		return Points;
	}

	const FTransform LandscapeTransform = Landscape->LandscapeActorToWorld();
	Points.Reserve(NumPoints);

	const FRandomStream RandomStream { Seed };
	FBox NewBounds = InBound;
	FIntPoint OutSize;
	bool bIsSuccess = false;
	TArray<uint16> HeightData = GetHeightDataInBound(Landscape, NewBounds, OutSize, bIsSuccess);
	const FBox LocalBound = NewBounds.InverseTransformBy(LandscapeTransform);

	const float ZScale = LANDSCAPE_ZSCALE;

	if (bIsSuccess)
	{
		for (uint32 PointIdx = 0; PointIdx < NumPoints; ++PointIdx)
		{
			const int32 RandomX = RandomStream.RandRange(LocalBound.Min.X, LocalBound.Max.X);
			const int32 RandomY = RandomStream.RandRange(LocalBound.Min.Y, LocalBound.Max.Y);
			const float Height = (HeightData[(RandomY - LocalBound.Min.Y) * OutSize.X + (RandomX - LocalBound.Min.X)] - (std::numeric_limits<uint16>::max() / 2)) * ZScale;
			const FVector Point = FVector(RandomX, RandomY, Height);
			Points.Add(Point);
		}
	}
	
	return Points;
}
