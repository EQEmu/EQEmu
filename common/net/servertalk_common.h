#pragma once

#include "common/servertalk.h"

namespace EQ
{
	namespace Net
	{
		enum ServertalkPacketType
		{
			ServertalkClientHello = 1,
			ServertalkServerHello,
			ServertalkClientHandshake,
			ServertalkClientDowngradeSecurityHandshake,
			ServertalkMessage,
		};
	}
}
