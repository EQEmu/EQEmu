/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

static constexpr char OP_SessionRequest      = 0x01;
static constexpr char OP_SessionResponse     = 0x02;
static constexpr char OP_Combined            = 0x03;
static constexpr char OP_SessionDisconnect   = 0x05;
static constexpr char OP_KeepAlive           = 0x06;
static constexpr char OP_SessionStatRequest  = 0x07;
static constexpr char OP_SessionStatResponse = 0x08;
static constexpr char OP_Packet              = 0x09;
static constexpr char OP_Fragment            = 0x0d;
static constexpr char OP_OutOfOrderAck       = 0x11;
static constexpr char OP_Ack                 = 0x15;
static constexpr char OP_AppCombined         = 0x19;
static constexpr char OP_OutOfSession        = 0x1d;
