//
// Created by dannu on 4/21/2026.
//

#pragma once

#include "common/emu_versions.h"
#include <memory>

namespace Buff { class IBuff; }
namespace Message { class IMessage; }

// store all static functions for the different patches here
const std::shared_ptr<Buff::IBuff>& GetBuffComponent(EQ::versions::ClientVersion version);
const std::shared_ptr<Message::IMessage>& GetMessageComponent(EQ::versions::ClientVersion version);
