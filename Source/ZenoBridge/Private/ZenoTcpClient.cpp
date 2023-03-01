#include "ZenoTcpClient.h"

#include "Helper.h"
#include "Common/TcpSocketBuilder.h"
#include "3rd/msgpack.h"
#include "Common/UdpSocketBuilder.h"
#include "Common/UdpSocketReceiver.h"
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
		if (CurrentSocket->GetConnectionState() != SCS_Connected)
		{
			UE_LOG(LogZeno, Error, TEXT("Connection lost, Stopping client."));
			Stop();
			continue;
		}

		ReadPacketToBuffer();
		ProcessTcpBuffer();

	}
	return 0;
}

void UZenoTcpClient::Stop()
{
	RemoveSessionFromZeno();
	bIsThreadStop = true;
	if (nullptr != CurrentSocket)
	{
		CurrentSocket->Close();
		FUnrealSocketHelper::DestroySocket(CurrentSocket);
		CurrentSocket = nullptr;
	}
	if (nullptr != UdpReceiver)
	{
		UdpReceiver->Stop();
		delete UdpReceiver;
		UdpReceiver = nullptr;
	}
	if (nullptr != UdpSocket)
	{
		UdpSocket->Close();
		FUnrealSocketHelper::DestroySocket(UdpSocket);
		UdpSocket = nullptr;
	}
}

void UZenoTcpClient::Exit()
{
	// if (nullptr != CurrentSocket)
	// {
	// 	CurrentSocket->Close();
	// 	FUnrealSocketHelper::DestroySocket(CurrentSocket);
	// }
	// if (nullptr != UdpReceiver)
	// {
	// 	UdpReceiver->Stop();
	// 	delete UdpReceiver;
	// }
	// if (nullptr != UdpSocket)
	// {
	// 	UdpSocket->Close();
	// 	FUnrealSocketHelper::DestroySocket(UdpSocket);
	// }
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
	bool bHasPending;
	CurrentSocket->WaitForPendingConnection(bHasPending, { 0, 0, 3});
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

void UZenoTcpClient::ProcessTcpBuffer()
{
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

void UZenoTcpClient::OnUdpDataReceived(const FArrayReaderPtr& Data, const FIPv4Endpoint& Endpoint)
{
	unsigned char* RawData = Data->GetData();
	const uint16 DataSize = Data->TotalSize();

	if (DataSize < sizeof(ZBUFileMessageHeader))
	{
		return;
	}

#ifdef PLATFORM_LITTLE_ENDIAN
	ZBFileType FileType = *reinterpret_cast<ZBFileType*>(RawData);
#else // PLATFORM_BIG_ENDIAN
	ZBFileType FileType = ByteSwap(*reinterpret_cast<ZBFileType*>(RawData));
#endif // PLATFORM_LITTLE_ENDIAN
	if (FileType > ZBFileType::End)
	{
		return;
	}

	ZBUFileMessageHeader* MessageHeader = reinterpret_cast<ZBUFileMessageHeader*>(RawData);
#ifndef PLATFORM_LITTLE_ENDIAN
	MessageHeader->type = ByteSwap(MessageHeader->type);
	MessageHeader->total_part = ByteSwap(MessageHeader->total_part);
	MessageHeader->size = ByteSwap(MessageHeader->size);
	MessageHeader->part_id = ByteSwap(MessageHeader->part_id);
	MessageHeader->file_id = ByteSwap(MessageHeader->file_id);
#endif // !PLATFORM_LITTLE_ENDIAN
	const uint32 FileId = MessageHeader->file_id;
	UE_LOG(LogZeno, Warning, TEXT("UDP File Transfer from '%ls'. FileID %d, PartID %d, Size %d."), *Endpoint.ToString(), FileId, MessageHeader->part_id, Data->Num());
	std::vector<uint8> MessageData;
	MessageData.resize(DataSize - sizeof(ZBUFileMessageHeader));
	std::memmove(MessageData.data(), RawData + sizeof(ZBUFileMessageHeader), DataSize - sizeof(ZBUFileMessageHeader));
	ZBUFileMessage Message {
		*MessageHeader,
		std::move(MessageData),
	};
	FileMessages.Push(MoveTemp(Message));

	TryMakeupFile(FileId);
}

void UZenoTcpClient::RemoveSessionFromZeno()
{
	if (!FUnrealSocketHelper::SessionName.IsSet()) return;
	ZPKRegisterSession Packet { FUnrealSocketHelper::SessionName.GetValue() };
	const auto PacketData = msgpack::pack(Packet);
	auto _ = SendPacket(ZBTControlPacketType::RemoveSession, PacketData.data(), PacketData.size());
	FUnrealSocketHelper::SessionName.Reset();
	CurrentSocket->Wait(ESocketWaitConditions::WaitForWrite, { 0, 0, 5 });
}

void UZenoTcpClient::TryMakeupFile(const uint32 FileId)
{
	TSet<uint16> PartSet;
	int32 FileParts = -1;
	ZBFileType FileType = ZBFileType::End;
	TMap<uint16, size_t> PartIdx;
	uint64 TotalSize = 0;
	for (size_t Idx = 0; Idx < FileMessages.Num(); ++Idx)
	{
		const ZBUFileMessage& Item1 = FileMessages[Idx];
		if (Item1.header.file_id == FileId)
		{
			FileParts = FMath::Max(static_cast<int32>(Item1.header.total_part), FileParts);
			PartSet.Add(Item1.header.part_id);
			FileType = Item1.header.type;
			PartIdx.Add(Item1.header.part_id, Idx);
			TotalSize += Item1.data.size();
		}
	}

	if (-1 != FileParts && PartSet.Num() == FileParts)
	{
		TArray<uint8> Data;
		Data.SetNumUninitialized(TotalSize, true);

		uint64 Offset = 0;

		for (size_t Idx = 0; Idx < FileParts; ++Idx)
		{
			std::vector<uint8>& RawData = FileMessages[Idx].data;
			std::memmove(Data.GetData() + Offset, RawData.data(), RawData.size());
			Offset += RawData.size();
		}

		OnNewFileNotifyDelegate.Broadcast(FileType, Data);
	}
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

		const FIPv4Endpoint Endpoint { {0}, 0 };
		UdpSocket = FUdpSocketBuilder(TEXT("ZenoBridgeUdpSocket"))
		   .AsNonBlocking()
		   .WithReceiveBufferSize(204800)
		   .BoundToEndpoint(Endpoint)
		;
		if (nullptr == UdpSocket)
		{
			UE_LOG(LogZeno, Error, TEXT("Failed to create UdpSocket."));
			return false;
		}
	}
	
	UdpSocket->GetAddress(OutEndpoint);

	UE_LOG(LogZeno, Warning, TEXT("UDP Socket is listening on %ls."), *OutEndpoint.ToString(true));

	UdpReceiver = new FUdpSocketReceiver(UdpSocket, FTimespan::Zero(), TEXT("ZenoBridgeUdpReceiver"));
	UdpReceiver->OnDataReceived().BindUObject(this, &UZenoTcpClient::OnUdpDataReceived);
	UdpReceiver->Start();

	return true;
}

bool UZenoTcpClient::IsRunning() const
{
	return !bIsThreadStop.load() && nullptr != CurrentSocket;
}
