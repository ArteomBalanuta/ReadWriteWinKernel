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
// Define an Interface Guid so that app can find the device and talk to it.
//

DEFINE_GUID (GUID_DEVINTERFACE_dummy2,
    0xf1d90757,0x75e4,0x417b,0xa3,0x55,0x01,0xc1,0x01,0xeb,0xcc,0xcd);
// {f1d90757-75e4-417b-a355-01c101ebcccd}
