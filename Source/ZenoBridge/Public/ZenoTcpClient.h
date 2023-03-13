#pragma once

#define DEFAULT_RECEIVE_BUFFER_SIZE 1440
#define DEFAULT_SEND_BUFFER_SIZE 2048

#include "CoreMinimal.h"
#include "Common/UdpSocketReceiver.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "model/bytebuffer.h"
#include "ZenoTcpClient.generated.h"

const auto GTextZenoSocketName = TEXT("ZenoTcpSocket");

DECLARE_MULTICAST_DELEGATE_TwoParams(FZenoClientNewFileNotify, const ZBFileType, const TArray<uint8>&);

class UZenoTcpClient;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FClientLostConnectionDelegate, UZenoTcpClient*, Thread);

UCLASS()
class UZenoTcpClient : public UObject, public FRunnable
{
	GENERATED_BODY()
	
public:
	using FByteBuffer = ByteBuffer<4096>;
	
	// ~start FRunnable interface
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;
	// ~end FRunnable interface

	void StartClient();
	void Setup(const FIPv4Endpoint& InBindingAddr);
	
	bool CreateRandomUDPSocket(FInternetAddr& OutEndpoint);
	bool IsRunning() const;

private:
	void ReadPacketToBuffer();
	bool SendPacket(const ZBTControlPacketType PacketType, const uint8* Data, const uint16 Size) const;

	void ProcessTcpBuffer();

	void OnUdpDataReceived(const FArrayReaderPtr& Data, const FIPv4Endpoint& Endpoint);

	void RemoveSessionFromZeno();

	void TryMakeupFile(const uint32 FileId);

	void SendUdpDatagrams(const TSharedRef<FInternetAddr>& Addr, const TArray<TArray<uint8>>& Datagrams) const;
	
public:
	FClientLostConnectionDelegate LostConnectionDelegate;
	FZenoClientNewFileNotify OnNewFileNotifyDelegate;

private:
	FIPv4Endpoint BindingAddr;

	FCriticalSection CriticalSection;

	FSocket* CurrentSocket = nullptr;

	std::atomic<bool> bIsThreadStop;

	TSet<FName> EncounteredSubjects;

	FByteBuffer ByteBuffer;

	FSocket* UdpSocket = nullptr;
	FUdpSocketReceiver* UdpReceiver = nullptr;

	TArray<ZBUFileMessage> FileMessages;
};
