#pragma once

#include "model/packethandler.h"
#include "Helper.h"
#include "3rd/msgpack.h"

REG_PACKET_HANDLER(AuthRequire, ZBTControlPacketType::AuthRequire, {
	bHasRespond	= true;
	outPacketType = ZBTControlPacketType::SendAuthToken;

	ZPKSendToken Packet { FUnrealSocketHelper::AuthToken.Get("") };
	auto Data = msgpack::pack(Packet);
	
	outSize = Data.size();
	outBuffer = std::move(Data);
});
