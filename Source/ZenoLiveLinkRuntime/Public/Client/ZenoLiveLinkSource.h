#pragma once
#include "ILiveLinkSource.h"

struct FZenoLiveLinkSetting;

/**
 * @brief Live link source is using to provide information to Unreal's live link module.
 */
class ZENOLIVELINKRUNTIME_API FZenoLiveLinkSource : public ILiveLinkSource, public TSharedFromThis<FZenoLiveLinkSource>
{
public:
	FZenoLiveLinkSource();

	// ~Impl ILiveLinkSource Start
	virtual void ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid) override;
	virtual bool IsSourceStillValid() const override;
	virtual bool RequestSourceShutdown() override;
	virtual void Update() override;
	// ~Impl ILiveLinkSource End

protected:
	static FZenoLiveLinkSetting& GetMutableConnectionSetting();
	static const FZenoLiveLinkSetting& GetConnectionSetting();

protected:
	/**
	 * @brief Texts to display on live link ui.
	 */
	FText SourceType;
	FText SourceMachineName;
	FText SourceStatus;

	/** Provide live link context */
	ILiveLinkClient* Client;
	/** Guid of this source */
	FGuid SourceGuid;

	/**
	 * @brief Name of subjects from zeno
	 */
	TSet<FName> EncounteredSubjects;
	
};
