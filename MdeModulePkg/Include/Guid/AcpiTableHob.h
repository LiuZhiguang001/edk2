/** @file
 Define the GUID gPldAcpiTableGuid HOB struct.

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _ACPI_TABLE_HOB_H_
#define _ACPI_TABLE_HOB_H_

#include <Uefi.h>

typedef struct {
  EFI_PHYSICAL_ADDRESS          Rsdp;
} PLD_ACPI_TABLE_HOB;

extern EFI_GUID gPldAcpiTableGuid;

#endif