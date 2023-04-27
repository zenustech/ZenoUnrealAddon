#include "Landscape/ZenoPCGVolumeComponent.h"
#include <zeno/unreal/ZenoRemoteTypes.h>

#include "LandscapeEdit.h"
#include "LandscapeProxy.h"
#include "Landscape/ZenoLandscapeHelper.h"

UZenoPCGVolumeComponent::UZenoPCGVolumeComponent(const FObjectInitializer& InObjectInitializer)
{
}

void UZenoPCGVolumeComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UZenoPCGVolumeComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

TSharedPtr<zeno::remote::HeightField> UZenoPCGVolumeComponent::GetLandscapeHeightData() const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UZenoPCGVolumeComponent::GetLandscapeHeightData);
	const AActor* Actor = GetOwner();
	if (!IsValid(Actor))
	{
		return nullptr;
	}
	
	FVector Origin;
	FVector Extent;
	Actor->GetActorBounds(false, Origin, Extent);
	const FBox ActorBounds = FBox::BuildAABB(Origin, Extent);

	TArray<TWeakObjectPtr<ALandscapeProxy>> Landscapes = Zeno::Helper::GetLandscapeProxies(Actor->GetWorld(), ActorBounds);

	if (Landscapes.IsEmpty())
	{
		return nullptr;
	}

	TSharedRef<zeno::remote::HeightField> Result = MakeShared<zeno::remote::HeightField>();
	TArray<uint16> HeightData;
	// TODO [darc] : Supporting multiple landscapes :
	TWeakObjectPtr<ALandscapeProxy> LandscapeProxy = Landscapes[0];
	if (LandscapeProxy.IsValid()) {}
	FLandscapeEditDataInterface EditDataInterface(LandscapeProxy->GetLandscapeInfo(), false);
	int32 MinX = static_cast<int32>(ActorBounds.Min.X), MinY = static_cast<int32>(ActorBounds.Min.Y), MaxX = static_cast<int32>(ActorBounds.Max.X), MaxY = static_cast<int32>(ActorBounds.Max.Y);
	const int32 VertsX = MaxX - MinX + 1, VertsY = MaxY - MinY + 1;
	HeightData.AddZeroed(VertsX * VertsY);
	EditDataInterface.GetHeightData(MinX, MinY, MaxX, MaxY, HeightData.GetData(), 0);

	if (MinX > MaxX || MinY > MaxY)
	{
		return Result;
	}
	
	Result->Data.resize(VertsY);
	for (int32 X = 0; X < VertsX; ++X)
	{
		auto& Vec = Result->Data[X];
		Vec.resize(VertsY);
		for (int32 Y = 0; Y < VertsY; ++Y)
		{
			Vec[Y] = HeightData[X + Y * VertsX];
		}
	}
	Result->Nx = VertsX;
	Result->Ny = VertsY;

	return Result;
}
