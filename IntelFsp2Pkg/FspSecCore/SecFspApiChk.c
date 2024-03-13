/** @file

  Copyright (c) 2016 - 2022, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "SecFsp.h"
#include "Guid/FspHeaderFile.h"
#include <Library/PeCoffLib.h>

/**
  Relocate Pe/Te Image
**/
EFI_STATUS
RelocatePeTeImage (
  UINT64  ImageBaseAddress
  )
{
  RETURN_STATUS                 Status;
  PE_COFF_LOADER_IMAGE_CONTEXT  ImageContext;

  ZeroMem (&ImageContext, sizeof (ImageContext));

  ImageContext.Handle    = (VOID *)ImageBaseAddress;
  ImageContext.ImageRead = PeCoffLoaderImageReadFromMemory;

  Status = PeCoffLoaderGetImageInfo (&ImageContext);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  ImageContext.ImageAddress = (EFI_PHYSICAL_ADDRESS)(UINTN)ImageBaseAddress;

  //
  // rebase the image
  //
  Status = PeCoffLoaderRelocateImage (&ImageContext);

  ASSERT_EFI_ERROR (Status);
  return Status;
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
  Status           = RelocatePeTeImage (SecCoreImageBase);
  if (!EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Sec Core is relocated successfully\n"));
  } else {
    DEBUG ((DEBUG_WARN, "Sec Core is not relocated. May have issue later\n"));
  }

  //
  // Get PeiCore image, and rebase it
  //
  PeiCoreImageBase = AsmGetRuntimePeiCoreAddress ();
  Status           = RelocatePeTeImage (PeiCoreImageBase);
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
