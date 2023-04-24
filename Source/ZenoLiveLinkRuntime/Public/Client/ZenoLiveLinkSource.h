#pragma once
#include "ILiveLinkSource.h"

class UZenoLiveLinkSession;
struct FZenoLiveLinkSetting;

/**
 * @brief Live link source is using to provide information to Unreal's live link module.
 */
class ZENOLIVELINKRUNTIME_API FZenoLiveLinkSource : public ILiveLinkSource, public TSharedFromThis<FZenoLiveLinkSource>
{
public:
	FZenoLiveLinkSource();
	explicit FZenoLiveLinkSource(FGuid SessionId);

	// ~Impl ILiveLinkSource Start
	virtual void ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid) override;
	virtual bool IsSourceStillValid() const override;
	virtual bool RequestSourceShutdown() override;
	virtual void Update() override;

	virtual FText GetSourceStatus() const override;
	virtual FText GetSourceType() const override;
	virtual FText GetSourceMachineName() const override;
	// ~Impl ILiveLinkSource End

	UZenoLiveLinkSession* GetSession() const;

	/** Check does this source contain specified subject name */
	bool HasSubject(FName InName) const;

	void AsyncUpdateSubjectList();

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

	FGuid SessionGuid;

	FCriticalSection UpdateSubjectListLock;
	double LastUpdateSubjectListTime = .0;
	int32 CurrentHistoryIndex = 0;
	
};
