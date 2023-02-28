#include "ZenoTcpClient.h"

#include "Helper.h"
#include "Common/TcpSocketBuilder.h"
#include "3rd/msgpack.h"
#include "Handler/Handlers.h"
#include "model/packethandler.h"

bool UZenoTcpClient::Init()
{
	CurrentSocket = FTcpSocketBuilder(GTextZenoSocketName)
		.AsReusable()
		.AsBlocking()
		.WithReceiveBufferSize(DEFAULT_RECEIVE_BUFFER_SIZE)
		.WithSendBufferSize(DEFAULT_SEND_BUFFER_SIZE)
	;

	if (nullptr == CurrentSocket)
	{
		UE_LOG(LogZeno, Error, TEXT("Failed to build a new tcp socket."));
		return false;
	}

	if (const TSharedRef<FInternetAddr> InternetAddr =
		FUnrealSocketHelper::NewInternetAddr(BindingAddr.Address.ToString(), BindingAddr.Port);
		!CurrentSocket->Connect(*InternetAddr))
	{
		const ESocketErrors LastErr = FUnrealSocketHelper::GetLastError();
		UE_LOG(LogZeno, Error, TEXT("Failed to connect to zeno host with error code (%d).\nError description: %s"), LastErr, FUnrealSocketHelper::GetErrorDesc(LastErr));
		return false;
	}

	UE_LOG(LogZeno, Warning, TEXT("Connected to %ls"), *BindingAddr.ToString());
	return true;
}

uint32 UZenoTcpClient::Run()
{
	while (IsRunning())
	{
		// blocking read socket status for up to 5s
		if (CurrentSocket->GetConnectionState() != SCS_Connected)
		{
			UE_LOG(LogZeno, Error, TEXT("Connection lost, Stopping client."));
			Stop();
			continue;
		}
		CurrentSocket->Wait(ESocketWaitConditions::WaitForReadOrWrite, FTimespan(0, 0, 5));

		ReadPacketToBuffer();

		if (const int16_t PacketSize = ByteBuffer.getNextPacketSize(); PacketSize != -1 && PacketSize > sizeof(ZBTPacketHeader))
		{
			TArray<uint8> TmpBuf;
			TmpBuf.Reserve(PacketSize);
			ByteBuffer.readSinglePacket(TmpBuf.GetData());
			const ZBTPacketHeader* PacketHeader = reinterpret_cast<const ZBTPacketHeader*>(TmpBuf.GetData());

			bool bHasRespond = false;
			ZBTControlPacketType RespondPacketType = ZBTControlPacketType::Start;
			OutPacketBufferType RespondData;
			uint16 RespondSize = 0;

			PacketHandlerMap::get().tryCall(PacketHeader->type, TmpBuf.GetData(), PacketHeader->length, bHasRespond, RespondPacketType, RespondData, RespondSize);

			if (bHasRespond && RespondData.has_value())
			{
				SendPacket(RespondPacketType, RespondData->data(), RespondData->size());
			}
		}
	}
	return 0;
}

void UZenoTcpClient::Stop()
{
	bIsThreadStop = true;
	CurrentSocket->Close();
	FUnrealSocketHelper::DestroySocket(CurrentSocket);
	CurrentSocket = nullptr;

	if (nullptr != UdpSocket)
	{
		UdpSocket->Close();
		FUnrealSocketHelper::DestroySocket(UdpSocket);
		UdpSocket = nullptr;
	}
}

void UZenoTcpClient::Exit()
{
	if (nullptr != CurrentSocket)
	{
		CurrentSocket->Close();
		FUnrealSocketHelper::DestroySocket(CurrentSocket);
	}
	if (nullptr != UdpSocket)
	{
		UdpSocket->Close();
		FUnrealSocketHelper::DestroySocket(UdpSocket);
	}
}

void UZenoTcpClient::StartClient()
{
	FRunnableThread::Create(this, GTextZenoSocketName);
}

void UZenoTcpClient::Setup(const FIPv4Endpoint& InBindingAddr)
{
	BindingAddr = InBindingAddr;
}

void UZenoTcpClient::ReadPacketToBuffer()
{
	if (uint32 DataSize; nullptr != CurrentSocket && CurrentSocket->HasPendingData(DataSize) && ByteBuffer.moveCursor(DataSize))
	{
		int32 ReadCnt;
		CurrentSocket->Recv(*ByteBuffer, DataSize, ReadCnt);
	}
}

bool UZenoTcpClient::SendPacket(const ZBTControlPacketType PacketType, const uint8* Data, const uint16 Size) const
{
    static std::atomic<uint16_t> CurrentPacketIndex = 0;

#ifdef PLATFORM_LITTLE_ENDIAN
    ZBTPacketHeader PacketHeader {
        CurrentPacketIndex.fetch_add(1),
        Size,
        PacketType,
    };
#else // PLATFORM_BIG_ENDIAN
    ZBTPacketHeader PacketHeader {
        ByteSwap(CurrentPacketIndex.fetch_add(1)),
        ByteSwap(Size),
        ByteSwap(PacketType),
    };
#endif // PLATFORM_LITTLE_ENDIAN

	if (nullptr != CurrentSocket && CurrentSocket->GetConnectionState() == SCS_Connected)
	{
		if (Size != 0 && nullptr == Data) return false;

		int32 ByteSent;
		CurrentSocket->Send(reinterpret_cast<const uint8*>(&PacketHeader), sizeof(PacketHeader), ByteSent);
		if (Size != 0)
		{
			CurrentSocket->Send(Data, Size, ByteSent);
		}
#ifdef PLATFORM_LITTLE_ENDIAN
		decltype(g_packetSplit)& PacketSplit = g_packetSplit;
#else // PLATFORM_BIG_ENDIAN
		decltype(g_packetSplit) PacketSplit;
		std::reverse_copy(g_packetSplit.begin(), g_packetSplit.end(), PacketSplit.begin());
#endif // PLATFORM_LITTLE_ENDIAN
		CurrentSocket->Send(PacketSplit.data(), PacketSplit.size(), ByteSent);
		
		return true;
	}

	return false;
}

void UZenoTcpClient::OnTcpDataReceived(const TArray<uint8>& Data, const FIPv4Endpoint& Sender)
{
}

bool UZenoTcpClient::CreateRandomUDPSocket(FInternetAddr& OutEndpoint)
{
	if (nullptr == UdpSocket)
	{
		ISocketSubsystem* SocketSubsystem = FUnrealSocketHelper::GetSocketSubsystem();
		if (nullptr == SocketSubsystem)
		{
			UE_LOG(LogZeno, Error, TEXT("Failed to get instance of SocketSubsystem."));
			return false;
		}

		UdpSocket = SocketSubsystem->CreateSocket(NAME_DGram, TEXT("ZenoBridgeUdpSocket"), true);
		if (nullptr == UdpSocket)
		{
			UE_LOG(LogZeno, Error, TEXT("Failed to create UdpSocket."));
			return false;
		}

		const FIPv4Endpoint Endpoint { {0}, 0 };
		if (!UdpSocket->Bind(*Endpoint.ToInternetAddr()))
		{
			UE_LOG(LogZeno, Error, TEXT("Failed to bind UdpSocket on %ls."), *Endpoint.ToString());
			return false;
		}
	}
	
	UdpSocket->GetAddress(OutEndpoint);

	UE_LOG(LogZeno, Verbose, TEXT("UDP Socket is listening on %ls."), *OutEndpoint.ToString(true));
	
	return true;
}

bool UZenoTcpClient::IsRunning() const
{
	return !bIsThreadStop.load() && nullptr != CurrentSocket;
}
