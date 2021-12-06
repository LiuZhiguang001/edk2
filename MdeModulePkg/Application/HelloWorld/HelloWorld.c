/** @file
  This sample application bases on HelloWorld PCD setting
  to print "UEFI Hello World!" to the UEFI Console.

  Copyright (c) 2006 - 2018, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PciLib.h>
#include <Library/IoLib.h>

//
// String token ID of help message text.
// Shell supports to find help message in the resource section of an application image if
// .MAN file is not found. This global variable is added to make build tool recognizes
// that the help string is consumed by user and then build tool will add the string into
// the resource section. Thus the application can use '-?' option to show help message in
// Shell.
//
GLOBAL_REMOVE_IF_UNREFERENCED EFI_STRING_ID mStringHelpTokenId = STRING_TOKEN (STR_HELLO_WORLD_HELP_INFORMATION);

#define BLOCK_SIZE 5

typedef enum {
  BOARD_UP,
  BOARD_DOWN,
  BOARD_RIGHT,
  BOARD_LEFT
} BOARD;
typedef struct {
  EFI_GRAPHICS_OUTPUT_PROTOCOL          *GraphicsInterface;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL         *BltBuffer;
  UINTN                                 GroundXSize;
  UINTN                                 GroundYSize;
  UINTN                                 BlockdXSize;
  UINTN                                 BlockdYSize;
} BLT_BUFFER_INFO;

EFI_GRAPHICS_OUTPUT_BLT_PIXEL BackPixel = {
  0xff, 0xff, 0xff, 0
};

EFI_GRAPHICS_OUTPUT_BLT_PIXEL BoardPixel = {
  0xC0, 0xC0, 0xC0, 0
};


EFI_GRAPHICS_OUTPUT_BLT_PIXEL*
GetPixelFromBlockOffest (
  BLT_BUFFER_INFO                       *BltBufferInfo,
  UINTN                                 BlockX,
  UINTN                                 BlockY,
  UINTN                                 X,
  UINTN                                 Y
  )
{
  UINTN                                 PhysicalX;
  UINTN                                 PhysicalY;
  PhysicalX = BlockX * BLOCK_SIZE + X;
  PhysicalY = BlockY * BLOCK_SIZE + Y;
  return BltBufferInfo->BltBuffer + BltBufferInfo->GroundXSize * PhysicalY + PhysicalX;
}

VOID
PaintBlockBorder (
  BLT_BUFFER_INFO                       *BltBufferInfo,
  UINTN                                 BlockX,
  UINTN                                 BlockY,
  BOARD                                 Board,
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL         *Pixel
  )
{
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL         *PixelFromBuffer;
  UINTN Index;

  for (Index = 0; Index < BLOCK_SIZE; Index++) {
    PixelFromBuffer = GetPixelFromBlockOffest (
                        BltBufferInfo,
                        BlockX,
                        BlockY,
                        (Board ==BOARD_RIGHT ? BLOCK_SIZE - 1 : 
                         Board ==BOARD_LEFT  ? 0 : Index),
                        (Board ==BOARD_DOWN ? BLOCK_SIZE - 1 : 
                         Board ==BOARD_UP  ? 0 : Index)
                        );
    CopyMem(PixelFromBuffer, Pixel, sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));

  }
}

VOID
PaintBlock (
  BLT_BUFFER_INFO                       *BltBufferInfo,
  UINTN                                 BlockX,
  UINTN                                 BlockY,
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL         *Pixel
  )
{
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL         *PixelFromBuffer;
  UINTN                                 Index1;
  UINTN                                 Index2;
  for (Index1 = 0; Index1 < BLOCK_SIZE; Index1++) {
    for (Index2 = 0; Index2 < BLOCK_SIZE; Index2++) {
      PixelFromBuffer = GetPixelFromBlockOffest (
                          BltBufferInfo,
                          BlockX,
                          BlockY,
                          Index1,
                          Index2
                          );
      CopyMem(PixelFromBuffer, Pixel, sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
    }
  }
}

VOID
PaintBorder (
  BLT_BUFFER_INFO                       *BltBufferInfo,
  UINTN                                 BlockStartX,
  UINTN                                 BlockStartY,
  UINTN                                 SizeX,
  UINTN                                 SizeY,
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL         *Pixel
  )
{
  UINTN                                 Index;

  for (Index = 0; Index < SizeX+1; Index++) {
    PaintBlock (BltBufferInfo, BlockStartX + Index, BlockStartY, Pixel);
    PaintBlock (BltBufferInfo, BlockStartX + Index, BlockStartY + SizeY, Pixel);
  }
  for (Index = 0; Index < SizeY+1; Index++) {
    PaintBlock (BltBufferInfo, BlockStartX , BlockStartY + Index, Pixel);
    PaintBlock (BltBufferInfo, BlockStartX + SizeX, BlockStartY + Index, Pixel);
  }
}

VOID
PaintGround (
  BLT_BUFFER_INFO                       *BltBufferInfo,
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL         *Pixel
  )
{
  UINTN Index1;
  UINTN Index2;
  for (Index1 = 0; Index1 < BltBufferInfo->GroundXSize; Index1++) {
    for (Index2 = 0; Index2 < BltBufferInfo->GroundYSize; Index2++) {
      CopyMem(BltBufferInfo->BltBuffer + Index2 * BltBufferInfo->GroundXSize + Index1, Pixel, sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
      
    }
  }
}

VOID
Flush (
  BLT_BUFFER_INFO                       *BltBufferInfo
  )
{
  EFI_GRAPHICS_OUTPUT_PROTOCOL          *GraphicsInterface;
  GraphicsInterface = BltBufferInfo->GraphicsInterface;
  DEBUG ((DEBUG_ERROR, "GroundXSize=0x%lx \n", *(UINTN*)((UINT8*)BltBufferInfo->BltBuffer + 0x2A3C8)));
  
  GraphicsInterface->Blt(
    GraphicsInterface,
    BltBufferInfo->BltBuffer,
    EfiBltBufferToVideo,
    0,0,0,0,BltBufferInfo->GroundXSize, BltBufferInfo->GroundYSize,0);
}
EFI_STATUS
EFIAPI
InitWindows (
  VOID
  ) 
{
  EFI_GRAPHICS_OUTPUT_PROTOCOL          *GraphicsInterface;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL         *BltBuffer;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  *Info;
  UINTN                                 SizeOfInfo;
  BLT_BUFFER_INFO                       BltBufferInfo;
  //EFI_GRAPHICS_OUTPUT_BLT_PIXEL         Pixel;

  gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (void**)&GraphicsInterface);
  GraphicsInterface->QueryMode(GraphicsInterface, GraphicsInterface->Mode->Mode, &SizeOfInfo, &Info);
  BltBufferInfo.GroundXSize = Info->HorizontalResolution;
  BltBufferInfo.GroundYSize = Info->VerticalResolution;
  BltBufferInfo.BlockdXSize = BltBufferInfo.GroundXSize/BLOCK_SIZE;
  BltBufferInfo.BlockdYSize = BltBufferInfo.GroundYSize/BLOCK_SIZE;
  BltBufferInfo.GraphicsInterface = GraphicsInterface;
  DEBUG ((DEBUG_ERROR, "GroundXSize=%d GroundYSize=%d\n", BltBufferInfo.GroundXSize, BltBufferInfo.GroundYSize));
  DEBUG ((DEBUG_ERROR, "BlockdXSize=%d BlockdYSize=%d\n", BltBufferInfo.BlockdXSize, BltBufferInfo.BlockdYSize));
  gBS->AllocatePages(AllocateAnyPages, EfiBootServicesData, EFI_SIZE_TO_PAGES(sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL) * BltBufferInfo.GroundXSize * BltBufferInfo.GroundYSize), (EFI_PHYSICAL_ADDRESS *)&BltBuffer);
  ZeroMem (BltBuffer, sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL) * BltBufferInfo.GroundXSize * BltBufferInfo.GroundYSize);
  BltBufferInfo.BltBuffer = BltBuffer;
  DEBUG ((DEBUG_ERROR, "BltBuffer=0x%p \n", BltBuffer));




  //GraphicsInterface->Blt(
  //  GraphicsInterface,
  //  BltBuffer,
  //  EfiBltVideoFill,
  //  0,0,0,0,BltBufferInfo.GroundXSize, BltBufferInfo.GroundYSize,0);
  PaintGround (&BltBufferInfo, &BackPixel);
  PaintBlockBorder (&BltBufferInfo, 10, 2,  BOARD_UP,    &BoardPixel);
  PaintBlockBorder (&BltBufferInfo, 3, 2,  BOARD_DOWN,  &BoardPixel);
  PaintBlockBorder (&BltBufferInfo, 5, 2,  BOARD_RIGHT, &BoardPixel);
  PaintBlockBorder (&BltBufferInfo, 10, 10,  BOARD_LEFT,  &BoardPixel);

  PaintBorder(&BltBufferInfo, 30, 10,  100, 100,  &BoardPixel);
  Flush(&BltBufferInfo);

  //GraphicsInterface->Blt(
  //  GraphicsInterface,
  //  BltBuffer,
  //  EfiBltBufferToVideo,
  //  0,0,0,0,BltBufferInfo.GroundXSize, BltBufferInfo.GroundYSize,0);
//
  //Print (L"kuang: %d  %d  \n", Info->HorizontalResolution, Info->VerticalResolution);
  return EFI_SUCCESS;
}

/**
  The user Entry Point for Application. The user code starts with this function
  as the real entry point for the application.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINT32            Index;
  EFI_STATUS        Status;
  EFI_INPUT_KEY     Key;
  UINTN             EventIndex;
  Index = 0;

  //
  // Three PCD type (FeatureFlag, UINT32 and String) are used as the sample.
  //
  if (FeaturePcdGet (PcdHelloWorldPrintEnable)) {
    for (Index = 0; Index < PcdGet32 (PcdHelloWorldPrintTimes); Index ++) {
      //
      // Use UefiLib Print API to print string to UEFI console
      //
      Print ((CHAR16*)PcdGetPtr (PcdHelloWorldPrintString));
    }
  }
InitWindows (
  
  ) ;
  while (TRUE) {
    gBS->WaitForEvent (1, &gST->ConIn->WaitForKey, &EventIndex);
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    if (EFI_ERROR(Status)) {
      break;
    }
    Print(L"%c", Key.UnicodeChar);
  }
  gBS->Stall(1000000);

  return EFI_SUCCESS;
}
