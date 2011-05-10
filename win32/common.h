#ifndef	HID_WIN32_COMMON_H
#define	HID_WIN32_COMMON_H

extern "C"
{
#include <windows.h>
#include <setupapi.h>

#ifdef __GNUC__			// MinGW
#include <ddk/hidpi.h>
#include <ddk/hidsdi.h>
#else				// WinDDK
#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <hidpi.h>
#include <hidsdi.h>
#endif
}

#endif	// HID_WIN32_COMMON_H
