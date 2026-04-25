//
// Created by dannu on 4/21/2026.
//

#pragma once

#include "common/emu_versions.h"
#include <memory>

namespace Message { class IMessage; }

// store all static functions for the different patches here, this can return nullptr for unsupported patches
const std::unique_ptr<Message::IMessage>& GetMessageComponent(EQ::versions::ClientVersion version);
