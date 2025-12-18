#include "zone/client.h"
#include "zone/worldserver.h"

extern WorldServer worldserver;

void command_camerashake(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1) || !sep->IsNumber(2)) {
		c->Message(Chat::Red, "Usage: #camerashake [Duration (Milliseconds)] [Intensity (1-10)]");
		return;
	}

	auto duration = Strings::ToInt(sep->arg[1]);
	auto intensity = Strings::ToInt(sep->arg[2]);

	auto pack = new ServerPacket(ServerOP_CameraShake, sizeof(ServerCameraShake_Struct));
	ServerCameraShake_Struct *camera_shake = (ServerCameraShake_Struct *) pack->pBuffer;
	camera_shake->duration = duration;
	camera_shake->intensity = intensity;
	worldserver.SendPacket(pack);
	c->Message(
		Chat::White,
		fmt::format(
			"Sending camera shake to world with a duration of {} ({}) and an intensity of {}.",
			Strings::MillisecondsToTime(duration),
			duration,
			intensity
		).c_str()
	);
	safe_delete(pack);
}

