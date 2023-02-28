#pragma once

#include "model/packethandler.h"
#include "Helper.h"
#include "3rd/msgpack.h"
#include "ZenoBridge.h"
#include <system_error>

REG_PACKET_HANDLER(AuthRequire, ZBTControlPacketType::AuthRequire, {
	bHasRespond	= true;
	outPacketType = ZBTControlPacketType::SendAuthToken;

	ZPKSendToken Packet { FUnrealSocketHelper::AuthToken.Get("") };
	auto Data = msgpack::pack(Packet);
	
	outSize = Data.size();
	outBuffer = std::move(Data);
});

REG_PACKET_HANDLER(RegisterSession, ZBTControlPacketType::RegisterSession, {
	std::error_code err;
	
	const ZPKRegisterSession Data = msgpack::unpack<ZPKRegisterSession>(static_cast<const uint8*>(inData) + sizeof(
		ZBTPacketHeader), inSize, err);

	FUnrealSocketHelper::SessionName = Data.sessionName;

	const FZenoBridgeModule& ZenoBridgeModule = FModuleManager::Get().GetModuleChecked<FZenoBridgeModule>("ZenoBridge");
	const auto Client = ZenoBridgeModule.GetTcpClient();
	if (Client && Client->IsRunning())
	{
		const TSharedRef<FInternetAddr> Addr = FUnrealSocketHelper::NewInternetAddr();
		Client->CreateRandomUDPSocket(Addr.Get());
		
		bHasRespond = true;
		outPacketType = ZBTControlPacketType::BindUdpToSession;
		const FString TmpIpStr = Addr->ToString(false);
		std::string IpStr {TCHAR_TO_UTF8(*TmpIpStr)};

		ZPKBindUdpToSession Packet;
		Packet.sessionName = std::move(Data.sessionName);
		Packet.address = std::move(IpStr);
		Packet.port = Addr->GetPort();
		auto PacketData = msgpack::pack(Packet);

		outSize = PacketData.size();
		outBuffer = std::move(PacketData);
	}
});

