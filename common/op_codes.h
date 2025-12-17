#pragma once


static const char OP_SessionRequest	=	0x01;
static const char OP_SessionResponse	=	0x02;
static const char OP_Combined		=	0x03;
static const char OP_SessionDisconnect	=	0x05;
static const char OP_KeepAlive		=	0x06;
static const char OP_SessionStatRequest	=	0x07;
static const char OP_SessionStatResponse=	0x08;
static const char OP_Packet		=	0x09;
static const char OP_Fragment		=	0x0d;
static const char OP_OutOfOrderAck	=	0x11;
static const char OP_Ack		=	0x15;
static const char OP_AppCombined	=	0x19;
static const char OP_OutOfSession	=	0x1d;
