#include "Client/ZenoLiveLinkSource.h"

#include "ILiveLinkClient.h"
#include "Client/ZenoLiveLinkClientSubsystem.h"
#include "Client/ZenoLiveLinkSession.h"
#include "Client/Role/LiveLinkZenoRemoteRole.h"

#define LOCTEXT_NAMESPACE "FZenoLiveLinkSource"

FZenoLiveLinkSource::FZenoLiveLinkSource()
	: Client(nullptr)
{
	SourceStatus = LOCTEXT("SourceStatus_NoData", "No data");
	SourceType = LOCTEXT("SourceType_Zeno", "Zeno");
	SourceMachineName = FText::Format(LOCTEXT("ZenoSourceMachineName", "{0}:{1}"), FText::FromString(GetConnectionSetting().IPAddress), FText::AsNumber(GetConnectionSetting().HTTPPortNumber, &FNumberFormattingOptions::DefaultNoGrouping()));
}

FZenoLiveLinkSource::FZenoLiveLinkSource(const FGuid SessionId)
	: FZenoLiveLinkSource()
{
	SessionGuid = SessionId;
}

void FZenoLiveLinkSource::ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid)
{
	Client = InClient;
	SourceGuid = InSourceGuid;
}

bool FZenoLiveLinkSource::IsSourceStillValid() const
{
	return SessionGuid.IsValid();
}

bool FZenoLiveLinkSource::RequestSourceShutdown()
{
	// Sync with update task
	FScopeLock Lock { &UpdateSubjectListLock };
	UZenoLiveLinkClientSubsystem* Subsystem = GEngine->GetEngineSubsystem<UZenoLiveLinkClientSubsystem>();
	Subsystem->RequestCloseSession(SessionGuid);
	SessionGuid.Invalidate();
	return true;
}

void FZenoLiveLinkSource::Update()
{
	const double CurrentTime = FPlatformTime::Seconds();
	const double TimePassed = CurrentTime - LastUpdateSubjectListTime;
	if (TimePassed > GetConnectionSetting().UpdateInterval / 1000 && UpdateSubjectListLock.TryLock())
	{
		UpdateSubjectListLock.Unlock();
		LastUpdateSubjectListTime = CurrentTime;
		AsyncUpdateSubjectList();
	}
}

FText FZenoLiveLinkSource::GetSourceStatus() const
{
	return SourceStatus;
}

FText FZenoLiveLinkSource::GetSourceType() const
{
	return SourceType;
}

FText FZenoLiveLinkSource::GetSourceMachineName() const
{
	return SourceMachineName;
}

UZenoLiveLinkSession* FZenoLiveLinkSource::GetSession() const
{
	check(SessionGuid.IsValid());
	UZenoLiveLinkSession* Res = GEngine->GetEngineSubsystem<UZenoLiveLinkClientSubsystem>()->GetSession(SessionGuid);
	check(IsValid(Res));
	return Res;
}

bool FZenoLiveLinkSource::HasSubject(const FName InName) const
{
	return EncounteredSubjects.Contains(InName);
}

void FZenoLiveLinkSource::AsyncUpdateSubjectList()
{
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]
	{
		FScopeLock Lock { &UpdateSubjectListLock };
		const UZenoLiveLinkSession* Session = GetSession();
		const UZenoHttpClient::TAsyncResult<zeno::remote::Diff> FutureDiff = Session->GetClient()->GetDiffFromRemote(CurrentHistoryIndex);
		FutureDiff.WaitFor(FTimespan::FromSeconds(5)); // TODO [darc] : remove hardcoded timeout :
		if (!FutureDiff.IsReady())
		{
			UE_LOG(LogTemp, Error, TEXT("Http hasn't finished in 5 seconds, skipping sync."));
			return;
		}
		const TOptional<zeno::remote::Diff> Diff = FutureDiff.Get();
		if (!Diff.IsSet())
		{
			UE_LOG(LogTemp, Error, TEXT("Http request failed, skipping sync."));
			return;
		}
		
		for (const std::string& Name : Diff->data)
		{
			EncounteredSubjects.Add(FName { Name.c_str() });
		}
		CurrentHistoryIndex = Diff->CurrentHistory;

		// Push updated subject to live link
		for (const FName& Name : EncounteredSubjects)
		{
			FLiveLinkStaticDataStruct StaticData(FLiveLinkZenoDummyStaticData::StaticStruct());
			FLiveLinkFrameDataStruct FrameData(FLiveLinkZenoDummyFrameData::StaticStruct());
			Client->PushSubjectStaticData_AnyThread({ SourceGuid, Name }, ULiveLinkZenoDummyRole::StaticClass(), MoveTemp(StaticData));
			Client->PushSubjectFrameData_AnyThread({ SourceGuid, Name }, MoveTemp(FrameData));
		}
	});
}

FZenoLiveLinkSetting& FZenoLiveLinkSource::GetMutableConnectionSetting()
{
	return GEngine->GetEngineSubsystem<UZenoLiveLinkClientSubsystem>()->ConnectionSetting;
}

const FZenoLiveLinkSetting& FZenoLiveLinkSource::GetConnectionSetting()
{
	return GEngine->GetEngineSubsystem<UZenoLiveLinkClientSubsystem>()->ConnectionSetting;
}

#undef LOCTEXT_NAMESPACE
