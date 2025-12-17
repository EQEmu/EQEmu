/*	EQEMu: Everquest Server Emulator
	
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#pragma once

#include "common/types.h"

namespace EQ
{
	namespace textures {
		enum : int8 { textureInvalid = -1, textureBegin };

		enum TextureSlot : int8 {
			armorHead = textureBegin,
			armorChest,
			armorArms,
			armorWrist,
			armorHands,
			armorLegs,
			armorFeet,
			weaponPrimary,
			weaponSecondary,
			materialCount,
			materialInvalid = textureInvalid
		};

		enum TintSlot : int8 {
			tintHead = textureBegin,
			tintChest,
			tintArms,
			tintWrist,
			tintHands,
			tintLegs,
			tintFeet,
			tintCount,
			tintInvalid = textureInvalid
		};

		constexpr int format_as(TextureSlot slot) { return static_cast<int>(slot); }

		const int8 LastTexture = weaponSecondary;
		const int8 LastTintableTexture = tintFeet;

		struct Texture_Struct {
			uint32 Material;
			uint32 Unknown1;
			uint32 EliteModel;
			uint32 HerosForgeModel;
			uint32 Unknown2; // same as material?
		};

		struct InternalTexture_Struct {
			uint32 HerosForgeModel;
			uint32 Material;
			uint32 Color;
		};

		struct TextureMaterial_Struct {
			uint32 Material;
		};

		struct Tint_Struct {
			union {
				struct {
					uint8 Blue;
					uint8 Green;
					uint8 Red;
					uint8 UseTint; // if there's a tint, this is FF
				};
				uint32 Color;
			};
		};

	} /*textures*/

	struct TextureProfile {
		union {
			struct {
				textures::Texture_Struct Head;
				textures::Texture_Struct Chest;
				textures::Texture_Struct Arms;
				textures::Texture_Struct Wrist;
				textures::Texture_Struct Hands;
				textures::Texture_Struct Legs;
				textures::Texture_Struct Feet;
				textures::Texture_Struct Primary;
				textures::Texture_Struct Secondary;
			};
			textures::Texture_Struct Slot[textures::materialCount];
		};
	};

	struct InternalTextureProfile {
		union {
			struct {
				textures::InternalTexture_Struct Head;
				textures::InternalTexture_Struct Chest;
				textures::InternalTexture_Struct Arms;
				textures::InternalTexture_Struct Wrist;
				textures::InternalTexture_Struct Hands;
				textures::InternalTexture_Struct Legs;
				textures::InternalTexture_Struct Feet;
				textures::InternalTexture_Struct Primary;
				textures::InternalTexture_Struct Secondary;
			};
			textures::InternalTexture_Struct Slot[textures::materialCount];
		};
	};

	struct TextureMaterialProfile {
		union {
			struct {
				textures::TextureMaterial_Struct Head;
				textures::TextureMaterial_Struct Chest;
				textures::TextureMaterial_Struct Arms;
				textures::TextureMaterial_Struct Wrist;
				textures::TextureMaterial_Struct Hands;
				textures::TextureMaterial_Struct Legs;
				textures::TextureMaterial_Struct Feet;
				textures::TextureMaterial_Struct Primary;
				textures::TextureMaterial_Struct Secondary;
			};
			textures::TextureMaterial_Struct Slot[textures::materialCount];
		};
	};

	struct TintProfile {
		union {
			struct {
				textures::Tint_Struct Head;
				textures::Tint_Struct Chest;
				textures::Tint_Struct Arms;
				textures::Tint_Struct Wrist;
				textures::Tint_Struct Hands;
				textures::Tint_Struct Legs;
				textures::Tint_Struct Feet;
				textures::Tint_Struct Primary;
				textures::Tint_Struct Secondary;
			};
			textures::Tint_Struct Slot[textures::materialCount]; // materialCount is correct..but, {[weaponPrimary],[weaponSecondary]} are not tintable...
		};
	};

} /*EQEmu*/
