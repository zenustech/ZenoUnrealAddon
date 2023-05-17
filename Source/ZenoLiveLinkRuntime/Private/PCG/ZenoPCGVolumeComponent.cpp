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

	TArray<TWeakObjectPtr<ALandscapeProxy>> Landscapes = Zeno::Helper::GetLandscapeProxies(Actor->GetWorld(), ActorBounds);

	if (Landscapes.IsEmpty())
	{
		return nullptr;
	}

	std::shared_ptr<zeno::remote::HeightField> Result = std::make_shared<zeno::remote::HeightField>();
	TArray<uint16> HeightData;
	// TODO [darc] : Supporting multiple landscapes :
	TWeakObjectPtr<ALandscapeProxy> LandscapeProxy = Landscapes[0];
	if (!LandscapeProxy.IsValid())
	{
		return nullptr;
	}

	FBox ActorBoundInLandscapeSpace = ActorBounds.TransformBy(LandscapeProxy->GetActorTransform());
	FTransform LandscapeTransform = LandscapeProxy->LandscapeActorToWorld();
	ActorBoundInLandscapeSpace = ActorBoundInLandscapeSpace.InverseTransformBy(LandscapeTransform);
	FVector LandscapeScale = LandscapeProxy->GetActorScale3D();
	FBox LandscapeBound = Zeno::Helper::GetLandscapeBounds(LandscapeProxy.Get());

	FLandscapeEditDataInterface EditDataInterface(LandscapeProxy->GetLandscapeInfo(), false);

	int32 MinX = FMath::FloorToInt(ActorBoundInLandscapeSpace.Min.X / LandscapeScale.X),
		MinY = FMath::FloorToInt(ActorBoundInLandscapeSpace.Min.Y / LandscapeScale.Y),
		MaxX = FMath::CeilToInt(ActorBoundInLandscapeSpace.Max.X / LandscapeScale.X),
		MaxY = FMath::CeilToInt(ActorBoundInLandscapeSpace.Max.Y / LandscapeScale.Y);

	MinX = FMath::Max(MinX, LandscapeBound.Min.X / LandscapeScale.X);
	MinY = FMath::Max(MinY, LandscapeBound.Min.Y / LandscapeScale.Y);
	MaxX = FMath::Min(MaxX, LandscapeBound.Max.X / LandscapeScale.X);
	MaxY = FMath::Min(MaxY, LandscapeBound.Max.Y / LandscapeScale.Y);
	
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
