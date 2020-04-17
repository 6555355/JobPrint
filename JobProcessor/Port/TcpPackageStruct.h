
#pragma once

enum TcpPackageCmd
{
	TcpPackageCmd_System = 0x00,			// ϵͳId

	TcpPackageCmd_PORTIDINFO = 0x01,

	TcpPackageCmd_LEFTOFFSET = 0x100,
	TcpPackageCmd_RIGHTOFFSET,
	TcpPackageCmd_MAP,
	TcpPackageCmd_BANDHEIGHT,
	TcpPackageCmd_JETENCODER,
	TcpPackageCmd_YADVANCE,
	TcpPackageCmd_MAXBANDINDEX,
};

enum System
{
	System_Yan1,
	System_Yan2_S,
	System_Yan2_A,
};