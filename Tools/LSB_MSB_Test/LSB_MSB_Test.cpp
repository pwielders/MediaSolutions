// LSB_MSB_Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int _tmain(int argc, _TCHAR* argv[])
{
	__int64 info = 0x0102030405060708LL;

	unsigned char list[8];

	::memcpy(&list[0], reinterpret_cast<unsigned char*>(&info), 8);

	return 0;
}

