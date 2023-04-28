#include "PCG/ZenoLandscapeHelper.h"

#if WITH_EDITOR
#include "Editor.h"
#endif // WITH_EDITOR
#include "Landscape.h"
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

	if (const ALandscape* Landscape = Cast<const ALandscape>(InLandscapeProxy))
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
