// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Volume.h"
#include "ZenoPCGVolume.generated.h"

class UZenoPCGVolumeComponent;

UCLASS(BlueprintType)
class ZENOLIVELINKRUNTIME_API AZenoPCGVolume : public AVolume
{
	GENERATED_BODY()

public:
	AZenoPCGVolume(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Zeno)
	UZenoPCGVolumeComponent* PCGComponent;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = Zeno)
	FString SubjectName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Zeno, AdvancedDisplay)
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

#if WITH_EDITOR
	virtual void PostActorCreated() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool GetReferencedContentObjects(TArray<UObject*>& Objects) const override;
	
	virtual void OnGeneratedNewMesh(FRawMesh& RawMesh, const FVector4f& InBoundDiff);
	virtual void SetStaticMeshComponent(UStaticMeshComponent* InStaticMeshComponent, const FVector4f& InBoundDiff);

	virtual void ExecutePCGGraph();
#endif // WITH_EDITOR

protected:
#if WITH_EDITOR
	void BuildBrush();
#endif // WITH_EDITOR
};

#if defined(UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_2)
#include "CoreMinimal.h"
#endif
