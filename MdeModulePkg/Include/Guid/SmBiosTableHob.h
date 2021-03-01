/** @file
 Define the GUID gPldSmbios3TableGuid and gPldSmbiosTableGuid HOB struct.

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _SMBIOS_TABLE_HOB_H_
#define _SMBIOS_TABLE_HOB_H_

#include <Uefi.h>

typedef struct {
  EFI_PHYSICAL_ADDRESS          SmBiosTableAddress;
} PLD_SMBIOS_TABLE_HOB;

extern EFI_GUID gPldSmbios3TableGuid;
extern EFI_GUID gPldSmbiosTableGuid;
#endif