#include "PCG/ZenoPCGVolumeComponent.h"
#include <zeno/unreal/ZenoRemoteTypes.h>

#include "LandscapeEdit.h"
#include "LandscapeProxy.h"
#include "ZenoGraphAsset.h"
#include "Components/BrushComponent.h"
#include "PCG/ZenoLandscapeHelper.h"
#include "PCG/ZenoPCGVolume.h"
#include "Utilities/ZenoEngineTypes.h"

UZenoPCGVolumeComponent::UZenoPCGVolumeComponent(const FObjectInitializer& InObjectInitializer)
{
}

void UZenoPCGVolumeComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UZenoPCGVolumeComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UZenoPCGVolumeComponent, ZenoGraph))
	{
		InputParameters.Reset();
		UZenoGraphAsset* GraphAsset = ZenoGraph.LoadSynchronous();
		for (const FZenoInputParameterDescriptor& Descriptor : GraphAsset->InputParameterDescriptors)
		{
			UZenoInputParameter* InputParameter = Descriptor.CreateInputParameter(this);
			if (nullptr != InputParameter)
			{
				InputParameters.Add(InputParameter);
			}
		}
	}
	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UZenoPCGVolumeComponent, InputParameters))
	{
		ParameterChangedEvent.Broadcast();
	}
	
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

std::shared_ptr<zeno::remote::HeightField> UZenoPCGVolumeComponent::GetLandscapeHeightData() const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UZenoPCGVolumeComponent::GetLandscapeHeightData);
	const AActor* Actor = GetOwner();
	if (!IsValid(Actor))
	{
		return nullptr;
	}
	
	FBox ActorBounds = GetActorBounds();

	TArray<TWeakObjectPtr<ALandscapeProxy>> Landscapes = Zeno::Helper::GetLandscapeProxies(Actor->GetWorld(), ActorBounds);

	if (Landscapes.IsEmpty())
	{
		return nullptr;
	}

	std::shared_ptr<zeno::remote::HeightField> Result = std::make_shared<zeno::remote::HeightField>();
	
	// TODO [darc] : Supporting multiple landscapes :
	ALandscapeProxy* LandscapeProxy = Landscapes[0].Get(false);
	if (LandscapeProxy == nullptr)
	{
        UE_LOG(LogTemp, Warning, TEXT("No Landscape found within Actor bounds."));
		return nullptr;
	}

	ActorBounds = ActorBounds.InverseTransformBy(LandscapeProxy->LandscapeActorToWorld());
	FVector LandscapeScale = LandscapeProxy->GetActorScale3D();
	FBox LandscapeBound = LandscapeProxy->GetComponentsBoundingBox().InverseTransformBy(LandscapeProxy->LandscapeActorToWorld());
	
	int32 MinX = FMath::FloorToInt((ActorBounds.Min.X));
	int32 MinY = FMath::FloorToInt((ActorBounds.Min.Y));
	int32 MaxX = FMath::CeilToInt((ActorBounds.Max.X));
	int32 MaxY = FMath::CeilToInt((ActorBounds.Max.Y));

	MinX = FMath::Clamp(MinX, 0, LandscapeBound.Min.X);
	MinY = FMath::Clamp(MinY, 0, LandscapeBound.Min.Y);
	MaxX = FMath::Clamp(MaxX, 0, LandscapeBound.Max.X);
	MaxY = FMath::Clamp(MaxY, 0, LandscapeBound.Max.Y);

	const size_t VertsX = MaxX - MinX + 1, VertsY = MaxY - MinY + 1;
	
	TArray<uint16> HeightData;
	HeightData.SetNumZeroed(VertsX * VertsY);
	
	FLandscapeEditDataInterface EditDataInterface(LandscapeProxy->GetLandscapeInfo());
	EditDataInterface.GetHeightData(MinX, MinY, MaxX, MaxY, HeightData.GetData(), 0);

	if (MinX > MaxX || MinY > MaxY)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Landscape data found."));
		return Result;
	}

	Result->Data.resize(VertsY);
	for (size_t Y = MinY; Y <= MaxY; ++Y)
	{
		for (size_t X = MinX; X <= MaxX; ++X)
		{
			size_t IdxX = X - MinX, IdxY = VertsY - (Y - MinY) - 1;
			uint16 Height = HeightData[IdxX + (IdxY * (MaxX - MinX + 1))];
			if (Result->Data[IdxY].size() == 0) Result->Data[IdxY].resize(VertsX);
			Result->Data[IdxY][IdxX] = Height;
		}
	}
	
	Result->Nx = VertsX;
	Result->Ny = VertsY;
	Result->LandscapeScale = LandscapeScale.Z;
	
	return Result;
}

std::shared_ptr<zeno::remote::PointSet> UZenoPCGVolumeComponent::GetScatteredPoints() const
{
	const AActor* Actor = GetOwner();
	if (!IsValid(Actor))
	{
		return nullptr;
	}

	const FBox ActorBounds = GetActorBounds();
	
	TArray<TWeakObjectPtr<ALandscapeProxy>> Landscapes = Zeno::Helper::GetLandscapeProxies(Actor->GetWorld(), ActorBounds);

	if (Landscapes.IsEmpty())
	{
		return nullptr;
	}
	
	ALandscapeProxy* LandscapeProxy = Landscapes[0].Get(false);

	TArray<FVector> Points = Zeno::Helper::ScatterPoints(LandscapeProxy, ScatterPoints, ScatterSeed, ActorBounds);

	std::shared_ptr<zeno::remote::PointSet> Result = std::make_shared<zeno::remote::PointSet>();
	Result->Points.reserve(Points.Num());
	for (const FVector& Point : Points)
	{
		Result->Points.emplace_back(zeno::remote::PCGPoint { zeno::remote::Vector3f { static_cast<float>(Point.X), static_cast<float>(Point.Y), static_cast<float>(Point.Z) } });
	}

	return Result;
}

FBox UZenoPCGVolumeComponent::GetActorBounds() const
{
	const AActor* Actor = GetOwner();
	if (!IsValid(Actor))
	{
		return FBox(ForceInit);
	}
	
	FBox ActorBounds;
	if (Actor->IsA<ALandscapeProxy>())
	{
		ActorBounds = Actor->GetComponentsBoundingBox(true);
	}
	else if (const AZenoPCGVolume* Volume = Cast<AZenoPCGVolume>(Actor); IsValid(Volume) && IsValid(Volume->GetBrushComponent()))
	{
		ActorBounds = Volume->GetBrushComponent()->Bounds.GetBox();
	}
	else
	{
		FVector Origin;
		FVector Extent;
		Actor->GetActorBounds(false, Origin, Extent);
		ActorBounds = FBox::BuildAABB(Origin, Extent);
	}

	return ActorBounds;
}
