// Fill out your copyright notice in the Description page of Project Settings.


#include "PCG/DataSource/ZenoPCGLandscapeData.h"

#include "LandscapeInfo.h"
#include "LandscapeProxy.h"
#include "PCG/ZenoLandscapeHelper.h"
#include "PCG/ZenoPCGSubsystem.h"
#include "PCG/Grid/ZenoPCGLandscapeCache.h"

void UZenoPCGLandscapeData::Initialize(const TArray<TWeakObjectPtr<ALandscapeProxy>>& InLandscapes,
                                       const FBox& InBounds, bool bInHeightOnly, bool bInUseMetadata)
{
	for (TWeakObjectPtr<ALandscapeProxy> InLandscape : InLandscapes)
	{
		if (InLandscape.IsValid())
		{
			Landscapes.Emplace(InLandscape.Get());
			LandscapeInfos.Emplace(Zeno::Helper::GetLandscapeBounds(InLandscape.Get()), InLandscape->GetLandscapeInfo());
		}
	}

	check(!Landscapes.IsEmpty());

	ALandscapeProxy* FirstLandscape = Landscapes[0].Get();
	check(IsValid(FirstLandscape));

	TargetActor = FirstLandscape;
	Bounds = InBounds;
	bHeightOnly = bInHeightOnly;
	bUseMetadata = bInUseMetadata;

	Transform = FirstLandscape->GetActorTransform();

	const UZenoPCGSubsystem* PcgSubsystem = UZenoPCGSubsystem::GetInstance(FirstLandscape->GetWorld());
	LandscapeCache = PcgSubsystem ? PcgSubsystem->GetLandscapeCache() : nullptr;
}

void UZenoPCGLandscapeData::PostLoad()
{
	Super::PostLoad();

	ALandscapeProxy* FirstLandscape = nullptr;

	for (TSoftObjectPtr<ALandscapeProxy> Landscape : Landscapes)
	{
		if (ALandscapeProxy* LandscapePtr = Landscape.LoadSynchronous())
		{
			LandscapeInfos.Emplace(Zeno::Helper::GetLandscapeBounds(LandscapePtr), LandscapePtr->GetLandscapeInfo());

			if (!FirstLandscape)
			{
				FirstLandscape = LandscapePtr;
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to load landscape %s"), *Landscape.ToString());
		}
	}

	TargetActor = FirstLandscape;
	
	const UZenoPCGSubsystem* PcgSubsystem = UZenoPCGSubsystem::GetInstance(FirstLandscape->GetWorld());
	LandscapeCache = PcgSubsystem ? PcgSubsystem->GetLandscapeCache() : nullptr;
}

FBox UZenoPCGLandscapeData::GetBounds() const
{
	return Bounds;
}

FBox UZenoPCGLandscapeData::GetStrictBounds() const
{
	return Bounds;
}

bool UZenoPCGLandscapeData::SamplePoint(const FTransform& InTransform, const FBox& InBounds, FZenoPCGPoint& OutPoint,
	UZenoMetadata* OutMetadata) const
{
	if (ProjectPoint(InTransform, InBounds, {}, OutPoint, OutMetadata))
	{
		if (InBounds.IsValid)
		{
			return FMath::PointBoxIntersection(OutPoint.Transform.GetLocation(), InBounds.TransformBy(InTransform));
		}
		else
		{
			return (InTransform.GetLocation() - OutPoint.Transform.GetLocation()).SquaredLength() < UE_SMALL_NUMBER;
		}
	}
	
	return false;
}

bool UZenoPCGLandscapeData::ProjectPoint(const FTransform& InTransform, const FBox& InBounds,
	const FZenoPCGProjectionParams& InParams, FZenoPCGPoint& OutPoint, UZenoMetadata* OutMetadata) const
{
	if (!LandscapeCache.IsValid())
	{
		return false;
	}

	const ULandscapeInfo* LandscapeInfo = GetLandscapeInfo(InTransform.GetLocation());
	if (!LandscapeInfo || !LandscapeInfo->GetLandscapeProxy())
	{
		return false;
	}

	const FTransform& LandscapeTransform = LandscapeInfo->GetLandscapeProxy()->GetTransform();

	const FVector LocalPoint = LandscapeTransform.InverseTransformPosition(InTransform.GetLocation());
	const FIntPoint ComponentMapKey(FMath::FloorToInt(LocalPoint.X / LandscapeInfo->ComponentSizeQuads), FMath::FloorToInt(LocalPoint.Y / LandscapeInfo->ComponentSizeQuads));

#if WITH_EDITOR
	ULandscapeComponent* LandscapeComponent = LandscapeInfo->XYtoComponentMap.FindRef(ComponentMapKey);
	const FZenoPCGLandscapeCacheEntry* LandscapeCacheEntry = LandscapeComponent ? LandscapeCache->GetCacheEntry(LandscapeComponent, ComponentMapKey) : nullptr;
#else
	const FZenoPCGLandscapeCacheEntry* LandscapeCacheEntry = LandscapeCache->GetCacheEntry(LandscapeInfo->LandscapeGuid, ComponentMapKey);
#endif // WITH_EDITOR

	if (!LandscapeCacheEntry)
	{
		return false;
	}

	const FVector2D ComponentLocalPoint = FVector2D(LocalPoint.X - ComponentMapKey.X * LandscapeInfo->ComponentSizeQuads, LocalPoint.Y - ComponentMapKey.Y * LandscapeInfo->ComponentSizeQuads);

	if (bHeightOnly)
	{
		LandscapeCacheEntry->GetInterpolatedPointHeightOnly(ComponentLocalPoint, OutPoint);
	}

	if (!InParams.bProjectPositions)
	{
		OutPoint.Transform.SetLocation(InTransform.GetLocation());
	}

	if (!InParams.bProjectRotations)
	{
		OutPoint.Transform.SetRotation(InTransform.GetRotation());
	}
	else
	{
		FVector RotVector = InTransform.GetRotation().ToRotationVector();
		RotVector.X = RotVector.Y = 0;
		OutPoint.Transform.SetRotation(OutPoint.Transform.GetRotation() * FQuat::MakeFromRotationVector(RotVector));
	}

	if (!InParams.bProjectScales)
	{
		OutPoint.Transform.SetScale3D(InTransform.GetScale3D());
	}
	
	return true;
}

const ULandscapeInfo* UZenoPCGLandscapeData::GetLandscapeInfo(const FVector& InPosition) const
{
	check(!LandscapeInfos.IsEmpty());

	if (LandscapeInfos.Num() == 1)
	{
		return LandscapeInfos[0].Value;
	}

	for (const TPair<FBox, ULandscapeInfo*>& InfoItem : LandscapeInfos)
	{
		if (InfoItem.Key.IsInsideXY(InPosition))
		{
			return InfoItem.Value;
		}
	}

	return nullptr;
}
