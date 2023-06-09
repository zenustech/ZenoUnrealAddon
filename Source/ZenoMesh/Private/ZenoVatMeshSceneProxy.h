#pragma once

struct FZenoVatMeshUniformData;
class FZenoVatMeshVertexFactory;

class FZenoVatMeshSceneProxy final : public FPrimitiveSceneProxy
{
public:
	FZenoVatMeshSceneProxy(const UPrimitiveComponent* InComponent, const FName& ResourceName = NAME_None);

	explicit FZenoVatMeshSceneProxy(FPrimitiveSceneProxy const& PrimitiveSceneProxy) = delete;

	virtual ~FZenoVatMeshSceneProxy() override;

	virtual SIZE_T GetTypeHash() const override;
	
	virtual uint32 GetMemoryFootprint() const override;

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override;

	virtual void CreateRenderThreadResources() override;
	
	virtual void DestroyRenderThreadResources() override;

private:
	FMaterialRelevance MaterialRelevance;

	TSoftObjectPtr<UMaterialInterface> MaterialInterface;

	FZenoVatMeshVertexFactory* VertexFactory = nullptr;

	FZenoVatMeshUniformData* UniformData = nullptr;
};
