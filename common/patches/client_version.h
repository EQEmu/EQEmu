//
// Created by dannu on 4/21/2026.
//

#pragma once

#include "common/emu_versions.h"
#include <memory>

#include "zone/client.h"

namespace ClientPatch {
class IBuff;
class IMessage;
}

// store all static functions for the different patches here
// store all static functions for the different patches here, this can return nullptr for unsupported patches
template <typename Component>
const std::unique_ptr<Component>& GetComponent(EQ::versions::ClientVersion version);

template <>
const std::unique_ptr<ClientPatch::IBuff>& GetComponent(EQ::versions::ClientVersion version);

template <>
const std::unique_ptr<ClientPatch::IMessage>& GetComponent(EQ::versions::ClientVersion version);

template <typename Component>
static Component* GetClientComponent(const Client* client)
{
    return GetComponent<Component>(client->GetClientVersion()).get();
}
