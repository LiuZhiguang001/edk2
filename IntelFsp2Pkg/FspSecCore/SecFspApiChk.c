/** @file

  Copyright (c) 2016 - 2022, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "SecFsp.h"
#include "Guid/FspHeaderFile.h"

/**
  Relocate Te Image
**/
EFI_STATUS
RelocateTeImage (
  UINT64  TeImageBaseAddress,
  UINT64  Adjust
  )
{
  EFI_TE_IMAGE_HEADER        *TeHdr;
  EFI_IMAGE_DATA_DIRECTORY   *RelocDir;
  EFI_IMAGE_BASE_RELOCATION  *RelocBase;
  EFI_IMAGE_BASE_RELOCATION  *RelocBaseEnd;
  UINT16                     *Reloc;
  UINT16                     *RelocEnd;
  CHAR8                      *FixupBase;
  CHAR8                      *Fixup;
  UINT16                     *F16;
  UINT32                     *F32;
  UINT64                     *F64;

  TeHdr = (EFI_TE_IMAGE_HEADER *)(UINTN)TeImageBaseAddress;

  if (TeHdr->Signature != EFI_TE_IMAGE_HEADER_SIGNATURE) {
    DEBUG ((DEBUG_WARN, "Warning: Not TE image\n"));
    return RETURN_UNSUPPORTED;
  }

  RelocDir  = &TeHdr->DataDirectory[0];
  RelocBase = (EFI_IMAGE_BASE_RELOCATION *)(UINTN)(
                                                   TeImageBaseAddress +
                                                   RelocDir->VirtualAddress +
                                                   sizeof (EFI_TE_IMAGE_HEADER) -
                                                   TeHdr->StrippedSize
                                                   );
  RelocBaseEnd = (EFI_IMAGE_BASE_RELOCATION *)((UINTN)RelocBase + (UINTN)RelocDir->Size - 1);

  //
  // Run the relocation information and apply the fixups
  //
  while (RelocBase < RelocBaseEnd) {
    Reloc     = (UINT16 *)((CHAR8 *)RelocBase + sizeof (EFI_IMAGE_BASE_RELOCATION));
    RelocEnd  = (UINT16 *)((CHAR8 *)RelocBase + RelocBase->SizeOfBlock);
    FixupBase = (CHAR8 *)(UINTN)(TeImageBaseAddress +
                                 RelocBase->VirtualAddress +
                                 sizeof (EFI_TE_IMAGE_HEADER) -
                                 TeHdr->StrippedSize
                                 );

    //
    // Run this relocation record
    //
    while (Reloc < RelocEnd) {
      Fixup = FixupBase + (*Reloc & 0xFFF);
      switch ((*Reloc) >> 12) {
        case EFI_IMAGE_REL_BASED_ABSOLUTE:
          break;

        case EFI_IMAGE_REL_BASED_HIGH:
          F16  = (UINT16 *)Fixup;
          *F16 = (UINT16)(*F16 + ((UINT16)((UINT32)Adjust >> 16)));
          break;

        case EFI_IMAGE_REL_BASED_LOW:
          F16  = (UINT16 *)Fixup;
          *F16 = (UINT16)(*F16 + (UINT16)Adjust);
          break;

        case EFI_IMAGE_REL_BASED_HIGHLOW:
          F32  = (UINT32 *)Fixup;
          *F32 = *F32 + (UINT32)Adjust;
          break;

        case EFI_IMAGE_REL_BASED_DIR64:
          F64  = (UINT64 *)Fixup;
          *F64 = *F64 + (UINT64)Adjust;
          break;

        default:
          //
          // Return the same EFI_UNSUPPORTED return since it does not recognize
          // other the relocation type.
          //
          return RETURN_UNSUPPORTED;
      }

      //
      // Next relocation record
      //
      Reloc += 1;
    }

    //
    // Next reloc block
    //
    RelocBase = (EFI_IMAGE_BASE_RELOCATION *)RelocEnd;
  }

  return EFI_SUCCESS;
}

/**
  This function check the FSP API calling condition.
**/
VOID
EFIAPI
FspApiPatch (
  )
{
  UINT64           FspBase;
  UINT64           SecCoreImageBase;
  UINT64           PeiCoreImageBase;
  FSP_INFO_HEADER  *FspInfoHeader;
  UINT64           Delta;
  EFI_STATUS       Status;

  FspBase       = AsmGetRuntimeFspBaseAddress ();
  FspInfoHeader = (FSP_INFO_HEADER *)(UINTN)AsmGetFspInfoHeader ();
  ASSERT (FspInfoHeader->Signature == FSP_INFO_HEADER_SIGNATURE);

  Delta = FspBase - (UINT64)FspInfoHeader->ImageBase;
  if (Delta == 0) {
    //
    // No need to patch FSP
    //
    return;
  }

  //
  // Fix up FspInfoHeader->ImageBase
  //
  FspInfoHeader->ImageBase = (UINT32)FspBase;
  if (FspInfoHeader->ImageBase != (UINT32)FspBase) {
    DEBUG ((DEBUG_WARN, "Current FSP area can not be changed. Maybe it is in flash\n"));
    return;
  }

  //
  // Get SecCore image, and rebase it
  //
  SecCoreImageBase = AsmGetRuntimeSecCoreAddress ();
  Status           = RelocateTeImage (SecCoreImageBase, Delta);
  if (!EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Sec Core is relocated successfully\n"));
  } else {
    DEBUG ((DEBUG_WARN, "Sec Core is not relocated. May have issue later\n"));
  }

  //
  // Get PeiCore image, and rebase it
  //
  PeiCoreImageBase = AsmGetRuntimePeiCoreAddress ();
  Status           = RelocateTeImage (PeiCoreImageBase, Delta);
  if (!EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Pei Core is relocated successfully\n"));
  } else {
    DEBUG ((DEBUG_INFO, "Pei Core is not relocated. May have issue later\n"));
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
