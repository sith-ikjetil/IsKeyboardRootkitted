/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    user and kernel

--*/

//
// Define an Interface Guid so that apps can find the device and talk to it.
//

DEFINE_GUID (GUID_DEVINTERFACE_IsKeyboardRootkittedDriver,
    0x676b766c, 0x5442, 0x4550, 0x8d, 0xb9, 0xff, 0xd5, 0x93, 0xbb, 0x3d, 0x70);
// {676b766c-5442-4550-8db9-ffd593bb3d70}

EXTERN_C_START

void WriteDebugInfo(char* pszInformation);

void WriteDebugInfoLine(char* pszInformation);

void WriteDebugInfoLineFormat(char* pszInformation, ...);

void FormatString(char* pszDest, size_t chLength, char* pszFormat, ...);
EXTERN_C_END