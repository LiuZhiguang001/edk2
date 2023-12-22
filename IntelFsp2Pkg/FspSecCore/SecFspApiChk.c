/** @file

  Copyright (c) 2016 - 2022, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "SecFsp.h"
#include "Guid/FspHeaderFile.h"

/**
  This function check the FSP API calling condition.
**/
VOID
EFIAPI
FspApiPatch (
  )
{
  UINT64           FspBase;
  FSP_INFO_HEADER  *FspInfoHeader;
  UINT32           Delta;
  FSP_PATCH_TABLE  *FspPatchTable;
  FSP_PATCH_DATA   *PatchOffset;
  UINTN            Index;
  UINT32           Offset;
  UINT32           *Value;

  FspBase       = AsmGetRuntimeFspBaseAddress ();
  DEBUG ((DEBUG_INFO, "AsmGetRuntimeFspBaseAddress:%lx\n", (UINT32)FspBase));
  FspInfoHeader = (FSP_INFO_HEADER *)(UINTN)AsmGetFspInfoHeader ();
  Delta         = (UINT32)(FspBase - FspInfoHeader->ImageBase);
  if (Delta == 0) {
    return;
  }

  ASSERT (FspInfoHeader->Signature == FSP_INFO_HEADER_SIGNATURE);
  FspPatchTable = (FSP_PATCH_TABLE *)FspInfoHeader;
  while (TRUE) {
    if (FspPatchTable->Signature == FSP_FSPP_SIGNATURE) {
      break;
    }

    FspPatchTable = (FSP_PATCH_TABLE *)(((UINTN)FspPatchTable) + FspPatchTable->HeaderLength);
  }

  DEBUG ((DEBUG_INFO, "Found FSPP, Delta = %x, count:%d\n", Delta, FspPatchTable->PatchEntryNum));
  PatchOffset = (FSP_PATCH_DATA   *)(FspPatchTable+1);
  for (Index = 0; Index < FspPatchTable->PatchEntryNum; Index++) {
    if ((PatchOffset->Bits.Type == 0) || (PatchOffset->Bits.Type == 0xF)) {
      if (PatchOffset->Bits.Reversed == 0) {
        Offset = PatchOffset->Bits.Offset;
      } else {
        Offset = FspInfoHeader->ImageSize - (0x1000000 - PatchOffset->Bits.Offset);
      }

      ASSERT (Offset < FspInfoHeader->ImageSize);

      Value   = (UINT32 *)(FspBase + Offset);
      *Value += Delta;
    }

    PatchOffset += 1;
  }
}

/**
  This function check the FSP API calling condition.

  @param[in]  ApiIdx           Internal index of the FSP API.
  @param[in]  ApiParam         Parameter of the FSP API.

**/
EFI_STATUS
EFIAPI
FspApiCallingCheck (
  IN UINT8  ApiIdx,
  IN VOID   *ApiParam
  )
{
  EFI_STATUS       Status;
  FSP_GLOBAL_DATA  *FspData;

  Status  = EFI_SUCCESS;
  FspData = GetFspGlobalDataPointer ();

  if (ApiIdx == NotifyPhaseApiIndex) {
    //
    // NotifyPhase check
    //
    if ((FspData == NULL) || ((UINTN)FspData == MAX_ADDRESS) || ((UINTN)FspData == MAX_UINT32)) {
      Status = EFI_UNSUPPORTED;
    } else {
      if (FspData->Signature != FSP_GLOBAL_DATA_SIGNATURE) {
        Status = EFI_UNSUPPORTED;
      }
    }
  } else if (ApiIdx == FspMemoryInitApiIndex) {
    //
    // FspMemoryInit check
    //
    if (((UINTN)FspData != MAX_ADDRESS) && ((UINTN)FspData != MAX_UINT32)) {
      Status = EFI_UNSUPPORTED;
    } else if (ApiParam == NULL) {
      Status = EFI_SUCCESS;
    } else if (EFI_ERROR (FspUpdSignatureCheck (ApiIdx, ApiParam))) {
      Status = EFI_INVALID_PARAMETER;
    }
  } else if (ApiIdx == TempRamExitApiIndex) {
    //
    // TempRamExit check
    //
    if ((FspData == NULL) || ((UINTN)FspData == MAX_ADDRESS) || ((UINTN)FspData == MAX_UINT32)) {
      Status = EFI_UNSUPPORTED;
    } else {
      if (FspData->Signature != FSP_GLOBAL_DATA_SIGNATURE) {
        Status = EFI_UNSUPPORTED;
      }
    }
  } else if ((ApiIdx == FspSiliconInitApiIndex) || (ApiIdx == FspMultiPhaseSiInitApiIndex)) {
    //
    // FspSiliconInit check
    //
    if ((FspData == NULL) || ((UINTN)FspData == MAX_ADDRESS) || ((UINTN)FspData == MAX_UINT32)) {
      Status = EFI_UNSUPPORTED;
    } else {
      if (FspData->Signature != FSP_GLOBAL_DATA_SIGNATURE) {
        Status = EFI_UNSUPPORTED;
      } else if (ApiIdx == FspSiliconInitApiIndex) {
        if (ApiParam == NULL) {
          Status = EFI_SUCCESS;
        } else if (EFI_ERROR (FspUpdSignatureCheck (FspSiliconInitApiIndex, ApiParam))) {
          Status = EFI_INVALID_PARAMETER;
        }

        //
        // Reset MultiPhase NumberOfPhases to zero
        //
        FspData->NumberOfPhases = 0;
      }
    }
  } else if (ApiIdx == FspMultiPhaseMemInitApiIndex) {
    if ((FspData == NULL) || ((UINTN)FspData == MAX_ADDRESS) || ((UINTN)FspData == MAX_UINT32)) {
      Status = EFI_UNSUPPORTED;
    }
  } else if (ApiIdx == FspSmmInitApiIndex) {
    //
    // FspSmmInitApiIndex check
    //
    if ((FspData == NULL) || ((UINTN)FspData == MAX_ADDRESS) || ((UINTN)FspData == MAX_UINT32)) {
      Status = EFI_UNSUPPORTED;
    } else {
      if (FspData->Signature != FSP_GLOBAL_DATA_SIGNATURE) {
        Status = EFI_UNSUPPORTED;
      } else if (ApiParam == NULL) {
        Status = EFI_SUCCESS;
      } else if (EFI_ERROR (FspUpdSignatureCheck (FspSmmInitApiIndex, ApiParam))) {
        Status = EFI_INVALID_PARAMETER;
      }
    }
  } else {
    Status = EFI_UNSUPPORTED;
  }

  if (!EFI_ERROR (Status)) {
    if ((ApiIdx != FspMemoryInitApiIndex)) {
      //
      // For FspMemoryInit, the global data is not valid yet
      // The API index will be updated by SecCore after the global data
      // is initialized
      //
      SetFspApiCallingIndex (ApiIdx);
    }
  }

  return Status;
}
