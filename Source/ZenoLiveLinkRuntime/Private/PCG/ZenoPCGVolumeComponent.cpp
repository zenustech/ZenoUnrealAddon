#include "PCG/ZenoPCGVolumeComponent.h"
#include <zeno/unreal/ZenoRemoteTypes.h>

#include "LandscapeDataAccess.h"
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

#if 0
	ActorBounds = ActorBounds.InverseTransformBy(LandscapeProxy->LandscapeActorToWorld());
	FVector LandscapeScale = LandscapeProxy->GetActorScale3D();
	FBox LandscapeBound = LandscapeProxy->GetComponentsBoundingBox().InverseTransformBy(LandscapeProxy->LandscapeActorToWorld());

	// ULandscapeInfo* Info = LandscapeProxy->GetLandscapeInfo();
	// FLandscapeEditDataInterface LandscapeEdit(Info);
	// auto& Components = LandscapeProxy->LandscapeComponents;
	// for (const TObjectPtr<ULandscapeComponent>& Component : Components)
	// {
	// 	if (IsValid(Component))
	// 	{
	// 		FLandscapeComponentDataInterface DataInterface(Component.Get());
	// 	}
	// }
	//
	// return nullptr;
	
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
	HeightData.SetNumUninitialized(VertsX * VertsY);
	for (size_t Index = 0; Index < HeightData.Num(); ++Index)
	{
		HeightData[Index] = 0x8000; // MidValue = 0x10000 / 2
	}
	
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
#else

	FIntPoint Size;
	
	TArray<uint16> HeightData = Zeno::Helper::GetHeightDataInBound(LandscapeProxy, ActorBounds, Size);
	
	Result->Data.resize(Size.Y);
	for (size_t Y = 0; Y < Size.Y; ++Y)
	{
		for (size_t X = 0; X < Size.X; ++X)
		{
			uint16 Height = HeightData[Y * Size.X + X];
			if (Result->Data[Y].size() == 0) Result->Data[Y].resize(Size.X);
			Result->Data[Y][X] = Height;
		}
	}
	
	Result->Nx = Size.X;
	Result->Ny = Size.Y;
	Result->LandscapeScaleX = LandscapeProxy->GetActorScale3D().X;
	Result->LandscapeScaleY = LandscapeProxy->GetActorScale3D().Y;
	Result->LandscapeScaleZ = LandscapeProxy->GetActorScale3D().Z;
#endif
	
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
		Result->Points.emplace_back(zeno::remote::PCGPoint { zeno::remote::Vector3f { static_cast<float>(Point.X), static_cast<float>(Point.Z), static_cast<float>(Point.Y) } });
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
