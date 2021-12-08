#include "snake.h"

#define BLOCK_SIZE 20
#define WAIT_TIME  5000
#define STRING_MAX_SIZE 200

CONST CHAR16 *StringTemplate = L"    Score:\n    %05d\n\n\nPress q to quit\nPress e to play\n wasd to control";

EFI_GRAPHICS_OUTPUT_BLT_PIXEL BackPixel = {
  0x0, 0x0, 0x0, 0
};
EFI_GRAPHICS_OUTPUT_BLT_PIXEL GameBackPixel = {
  0xDc, 0xDc, 0xDc, 0
};

EFI_GRAPHICS_OUTPUT_BLT_PIXEL BoardPixel = {
  0x80, 0x80, 0x80, 0
};

EFI_GRAPHICS_OUTPUT_BLT_PIXEL SnakePixel = {
   0x1E, 0x69, 0xD2, 0
};

EFI_GRAPHICS_OUTPUT_BLT_PIXEL FoodPixel = {
   0x7A, 0x96, 0xE9, 0
};

EFI_GRAPHICS_OUTPUT_BLT_PIXEL BlackPixel = {
  0x0, 0x0, 0x0, 0
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
PaintSnake (
  BLT_BUFFER_INFO                       *BltBufferInfo,
  SNAKE_INFO                            *SnakeInfo
  )
{
  UINTN                                 Index;
  for (Index = 0; Index < SnakeInfo->SnakeLength; Index++) {
    PaintBlock (
      BltBufferInfo,
      BltBufferInfo->GameStartX + (SnakeInfo->SnakeArry + Index)->X,
      BltBufferInfo->GameStartY + (SnakeInfo->SnakeArry + Index)->Y,
      &SnakePixel
    );
  }
}

VOID
PaintGameGround (
  BLT_BUFFER_INFO                       *BltBufferInfo,
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL         *Pixel
  )
{
  UINTN Index1;
  UINTN Index2;
  for (Index1 = 0; Index1 < BltBufferInfo->GameSizeX; Index1++) {
    for (Index2 = 0; Index2 < BltBufferInfo->GameSizeY; Index2++) {
      PaintBlock (
        BltBufferInfo,
        BltBufferInfo->GameStartX + Index1,
        BltBufferInfo->GameStartY + Index2,
        Pixel
      );
    }
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
  GraphicsInterface->Blt(
    GraphicsInterface,
    BltBufferInfo->BltBuffer,
    EfiBltBufferToVideo,
    0,0,0,0,BltBufferInfo->GroundXSize, BltBufferInfo->GroundYSize,0);
}

VOID
PaintStringOnScreen (
  BLT_BUFFER_INFO   *BltBufferInfo,
  SNAKE_INFO        *SnakeInfo
  )
{
  EFI_IMAGE_OUTPUT                    *Blt;
  EFI_STATUS                          Status;
  EFI_HII_FONT_PROTOCOL               *HiiFont;
  UINTN                               Index1;
  UINTN                               Index2;

  Blt     = BltBufferInfo->StringBlt;
  HiiFont = BltBufferInfo->HiiFont;
  
  ZeroMem(Blt->Image.Bitmap,  Blt->Width * Blt->Height * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
  ZeroMem(BltBufferInfo->StringBuffer, STRING_MAX_SIZE);

  UnicodeSPrint (
    BltBufferInfo->StringBuffer,
    STRING_MAX_SIZE,
    StringTemplate,
    SnakeInfo->Score
    );
  Status = HiiFont->StringToImage (
                        HiiFont,
                        EFI_HII_IGNORE_IF_NO_GLYPH,
                        BltBufferInfo->StringBuffer,
                        NULL, &Blt, 0, 0, NULL, NULL, NULL
                        );

  for (Index1 = 0; Index1 < Blt->Width; Index1++){
    for (Index2 = 0; Index2 < Blt->Height; Index2++){
      CopyMem(
        BltBufferInfo->BltBuffer + Index1 + BltBufferInfo->StringStartX + (Index2 + BltBufferInfo->StringStartY) * BltBufferInfo->GroundXSize,
        Blt->Image.Bitmap + Index1 + Index2 * Blt->Width,
        sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
      );
    }
  }
}

EFI_STATUS
EFIAPI
Initialization (
  BLT_BUFFER_INFO   *BltBufferInfo,
  SNAKE_INFO        *SnakeInfo
  ) 
{
  EFI_GRAPHICS_OUTPUT_PROTOCOL          *GraphicsInterface;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL         *BltBuffer;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  *Info;
  UINTN                                 SizeOfInfo;
  EFI_TIME                              TheTime;
  gRT->GetTime(&TheTime, NULL);
  RandomSeed ((UINT8 *)&TheTime, sizeof (TheTime));

  gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (void**)&GraphicsInterface);
  GraphicsInterface->QueryMode(GraphicsInterface, GraphicsInterface->Mode->Mode, &SizeOfInfo, &Info);
  BltBufferInfo->GroundXSize = Info->HorizontalResolution;
  BltBufferInfo->GroundYSize = Info->VerticalResolution;
  BltBufferInfo->BlockdXSize = BltBufferInfo->GroundXSize/BLOCK_SIZE;
  BltBufferInfo->BlockdYSize = BltBufferInfo->GroundYSize/BLOCK_SIZE;
  BltBufferInfo->GraphicsInterface = GraphicsInterface;
  gBS->AllocatePages(AllocateAnyPages, EfiBootServicesData, EFI_SIZE_TO_PAGES(sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL) * BltBufferInfo->GroundXSize * BltBufferInfo->GroundYSize), (EFI_PHYSICAL_ADDRESS *)&BltBuffer);
  ZeroMem (BltBuffer, sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL) * BltBufferInfo->GroundXSize * BltBufferInfo->GroundYSize);
  BltBufferInfo->BltBuffer = BltBuffer;
  // Horizontal:  BlockdYSize/10-1  1 BlockdYSize/10*8 1 (BlockdXSize - BlockdYSize/10 -2 - BlockdYSize/10*8)
  BltBufferInfo->GameSizeX = BltBufferInfo->BlockdYSize/10*8;
  BltBufferInfo->GameSizeY = BltBufferInfo->BlockdYSize/10*8;
  BltBufferInfo->GameStartX = BltBufferInfo->BlockdYSize/10;
  BltBufferInfo->GameStartY = BltBufferInfo->BlockdYSize/10;

  BltBufferInfo->BlockOccupied = AllocateZeroPool (BltBufferInfo->GameSizeY * BltBufferInfo->GameSizeX * sizeof(BOOLEAN));
  
  BltBufferInfo->StringBlt = (EFI_IMAGE_OUTPUT *) AllocateZeroPool (sizeof (EFI_IMAGE_OUTPUT));
  BltBufferInfo->StringStartX = (BltBufferInfo->GameStartX + BltBufferInfo->GameSizeX + 1) * BLOCK_SIZE + 30;
  BltBufferInfo->StringStartY = (BltBufferInfo->GameSizeX/2) * BLOCK_SIZE - 100;
  BltBufferInfo->StringBlt->Width        = (UINT16) (BltBufferInfo->GroundXSize - BltBufferInfo->StringStartX);
  BltBufferInfo->StringBlt->Height       = (UINT16) (BltBufferInfo->GroundYSize - BltBufferInfo->StringStartY);
  BltBufferInfo->StringBlt->Image.Bitmap = AllocateZeroPool ((UINT32) BltBufferInfo->StringBlt->Width * BltBufferInfo->StringBlt->Height * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
  BltBufferInfo->StringBuffer = AllocateZeroPool ((UINT32) 200);
  gBS->LocateProtocol (&gEfiHiiFontProtocolGuid, NULL, (VOID **) &(BltBufferInfo->HiiFont));

  gBS->AllocatePages(AllocateAnyPages, EfiBootServicesData, EFI_SIZE_TO_PAGES(sizeof(SNAKE_POINT) * BltBufferInfo->GameSizeX * BltBufferInfo->GameSizeY + 1), (EFI_PHYSICAL_ADDRESS *)&SnakeInfo->SnakeArry);
  return EFI_SUCCESS;
}


VOID
InitSnake (
  BLT_BUFFER_INFO   *BltBufferInfo,
  SNAKE_INFO        *SnakeInfo
  ) 
{
  UINTN  Index;
  ZeroMem (SnakeInfo->SnakeArry, sizeof(SNAKE_POINT) * BltBufferInfo->GameSizeX * BltBufferInfo->GameSizeY + 1);
  SnakeInfo->SnakeLength = 5;
  for (Index = 0; Index < SnakeInfo->SnakeLength; Index++) {
    (SnakeInfo->SnakeArry + Index)->X = BltBufferInfo->GameSizeX/2;
    (SnakeInfo->SnakeArry + Index)->Y = BltBufferInfo->GameSizeY/2 - 2 + Index;
  }
  SnakeInfo->SnakeOrientaion = SNAKE_UP;
  SnakeInfo->SnakeBackOrientaion = SNAKE_DOWN;
  SnakeInfo->Score = 0;
}

BOOLEAN
SnakeMove (
  BLT_BUFFER_INFO   *BltBufferInfo,
  SNAKE_INFO        *SnakeInfo,
  SNAKE_POINT       *Food,
  SNAKE_STATUS      *SnakeStatus
  )
{
  UINTN Index;
  UINTN CurrentX;
  UINTN CurrentY;
  *SnakeStatus = SNAKE_GOOD;
  CurrentX = SnakeInfo->SnakeArry->X;
  CurrentY = SnakeInfo->SnakeArry->Y;
  for (Index = SnakeInfo->SnakeLength; Index > 0; Index--) {
    CopyMem(SnakeInfo->SnakeArry + Index, SnakeInfo->SnakeArry + Index - 1, sizeof(SNAKE_POINT));
  }
  
  switch (SnakeInfo->SnakeOrientaion)
  {
  case SNAKE_UP:
    SnakeInfo->SnakeArry->Y = CurrentY - 1;
    SnakeInfo->SnakeBackOrientaion = SNAKE_DOWN;
    break;
  case SNAKE_DOWN:
    SnakeInfo->SnakeArry->Y = CurrentY + 1;
    SnakeInfo->SnakeBackOrientaion = SNAKE_UP;
    break;
  case SNAKE_LEFT:
    SnakeInfo->SnakeArry->X = CurrentX - 1;
    SnakeInfo->SnakeBackOrientaion = SNAKE_RIGHT;
    break;
  case SNAKE_RIGHT:
    SnakeInfo->SnakeArry->X = CurrentX + 1;
    SnakeInfo->SnakeBackOrientaion = SNAKE_LEFT;
    break;
  default:
    break;
  }

  if (SnakeInfo->SnakeArry->X >= BltBufferInfo->GameSizeX) {
    *SnakeStatus = SNAKE_HIT_BOARD;
  }

  if (SnakeInfo->SnakeArry->Y >= BltBufferInfo->GameSizeY) { 
    *SnakeStatus = SNAKE_HIT_BOARD;
  }
  
  if ((SnakeInfo->SnakeArry->X == Food->X) && (SnakeInfo->SnakeArry->Y == Food->Y)) {
    SnakeInfo->SnakeLength++;
  } else {
    for (Index = 1; Index < SnakeInfo->SnakeLength; Index++) {
      if ((SnakeInfo->SnakeArry->X == (SnakeInfo->SnakeArry + Index)->X) && (SnakeInfo->SnakeArry->Y == (SnakeInfo->SnakeArry + Index)->Y)) {
        *SnakeStatus = SNAKE_HIT_ITSELF;
        return FALSE;
      }
    }
  }

  PaintGameGround (BltBufferInfo, &GameBackPixel);

  PaintSnake (BltBufferInfo, SnakeInfo);
  if ((SnakeInfo->SnakeArry->X == Food->X) && (SnakeInfo->SnakeArry->Y == Food->Y)) {
    return TRUE;
  } else {
    return FALSE;
  }
  
}

SNAKE_POINT
GenerateFood (
  BLT_BUFFER_INFO   *BltBufferInfo,
  SNAKE_INFO        *SnakeInfo
  )
{
  UINTN             RandValue;
  UINTN             EmptySize;
  UINTN             CurrentIndex;
  UINTN             GoalIndex;
  UINTN             Index;
  SNAKE_POINT       Food;
  UINTN             Millisecond;
  UINTN             TimePoint;
  EFI_TIME          TheTime;

  gRT->GetTime(&TheTime, NULL);

  Millisecond = EfiTimeToEpoch (&TheTime) * 1000 + TheTime.Nanosecond / 1000;
  
  if (SnakeInfo->SnakeLength != 5) {
    //  
    //  The player will always get the point as the snake's length * snake's length
    //  if the time is less than 0.1s, player will get 5000 point, means 500/Millisecond
    //
    TimePoint = 500 * 1000 / Millisecond;
    if (TimePoint > 5000) {
      TimePoint = 5000;
    }
    SnakeInfo ->Score = SnakeInfo ->Score + SnakeInfo->SnakeLength * SnakeInfo->SnakeLength + TimePoint;
  }
  ZeroMem(BltBufferInfo->BlockOccupied, BltBufferInfo->GameSizeX * BltBufferInfo->GameSizeY);
  RandomBytes ((UINT8 *)&RandValue, sizeof(UINTN));
  EmptySize = BltBufferInfo->GameSizeX * BltBufferInfo->GameSizeY - SnakeInfo->SnakeLength;
  GoalIndex = RandValue % EmptySize;
  Food.X = 0;
  Food.Y = 0;
  CurrentIndex = 0;

  for (Index = 0; Index < SnakeInfo->SnakeLength; Index++) {
    CurrentIndex = (SnakeInfo->SnakeArry + Index)->X + (SnakeInfo->SnakeArry + Index)->Y * BltBufferInfo->GameSizeX;
    *(BltBufferInfo->BlockOccupied + CurrentIndex) = TRUE;
  }
  CurrentIndex = 0;
  while (TRUE) {
    while (*(BltBufferInfo->BlockOccupied + CurrentIndex))
    {
      CurrentIndex++;
    }
    if (GoalIndex == 0) {
      break;
    }
    GoalIndex--;
    CurrentIndex++;
  }
  Food.X = CurrentIndex % BltBufferInfo->GameSizeX;
  Food.Y = CurrentIndex / BltBufferInfo->GameSizeX;
  return Food;
}


VOID
SnakeRun (
  BLT_BUFFER_INFO   *BltBufferInfo,
  SNAKE_INFO        *SnakeInfo,
  SNAKE_STATUS      *SnakeStatus
  )
{
  EFI_STATUS        Status;
  EFI_INPUT_KEY     Key;
  SNAKE_ORIENTATION InputOrientatin;
  UINTN             Index;
  SNAKE_POINT       Food;
  BOOLEAN           NeedMoreFood;
  BOOLEAN           GameOver;

  GameOver     = FALSE;
  Index        = 0;
  NeedMoreFood = TRUE;
  Food = GenerateFood (BltBufferInfo, SnakeInfo);
  PaintBlock (
    BltBufferInfo,
    BltBufferInfo->GameStartX + Food.X,
    BltBufferInfo->GameStartY + Food.Y,
    &FoodPixel
  );
  Flush(BltBufferInfo);
  while (TRUE) {
    gBS->Stall(WAIT_TIME);
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    if (!EFI_ERROR(Status)) {
      switch (Key.UnicodeChar)
      {
      case 'w':
        InputOrientatin = SNAKE_UP;
        break;
      case 'd':
        InputOrientatin = SNAKE_RIGHT;
        break;
      case 's':
        InputOrientatin = SNAKE_DOWN;
        break;
      case 'a':
        InputOrientatin = SNAKE_LEFT;
        break;
      case 'q':
        *SnakeStatus = SNAKE_GAME_OVER;
        break;
      default:
        break;
      }
      if (*SnakeStatus == SNAKE_GAME_OVER) {
        break;
      }
      if (InputOrientatin != SnakeInfo->SnakeBackOrientaion) {
        if (InputOrientatin != SnakeInfo->SnakeOrientaion) {
          SnakeInfo->SnakeOrientaion = InputOrientatin;
        } else {
          Index += 20;
        }
      }
    }

    if (Index > 50) {
      Index = 0;
      NeedMoreFood = SnakeMove (
                       BltBufferInfo,
                       SnakeInfo,
                       &Food,
                       SnakeStatus
                       );
      if (*SnakeStatus != SNAKE_GOOD) {
        break;
      }
      if (NeedMoreFood) { 
        Food = GenerateFood (BltBufferInfo,SnakeInfo);
        PaintStringOnScreen (BltBufferInfo, SnakeInfo);
      }
      PaintBlock (
        BltBufferInfo,
        BltBufferInfo->GameStartX + Food.X,
        BltBufferInfo->GameStartY + Food.Y,
        &FoodPixel
      );
      Flush(BltBufferInfo);
    } else {
      Index++;
    }
  }
}

VOID
SnakeMain (
  VOID
  )
{
  EFI_STATUS        Status;
  EFI_INPUT_KEY     Key;
  UINTN             EventIndex;
  BLT_BUFFER_INFO   BltBufferInfo;
  SNAKE_INFO        SnakeInfo;
  BOOLEAN           NewGame;
  SNAKE_STATUS      SnakeStatus;

  SnakeStatus = SNAKE_GOOD;
  Initialization (&BltBufferInfo, &SnakeInfo);

  InitSnake (&BltBufferInfo, &SnakeInfo);
  PaintGround (&BltBufferInfo, &BackPixel);
  PaintBorder(&BltBufferInfo, BltBufferInfo.GameStartX - 1, BltBufferInfo.GameStartY - 1, BltBufferInfo.GameSizeX + 1, BltBufferInfo.GameSizeY + 1,  &BoardPixel);
  PaintGameGround (&BltBufferInfo, &GameBackPixel);
  PaintSnake (&BltBufferInfo, &SnakeInfo);
  PaintStringOnScreen (&BltBufferInfo, &SnakeInfo);
  Flush(&BltBufferInfo);
  while (TRUE) {
    NewGame = FALSE;
    while (TRUE) {
      if (SnakeStatus == SNAKE_GAME_OVER) {
        PaintGround (&BltBufferInfo, &BlackPixel);
        Flush(&BltBufferInfo);
        return;
      }
      gBS->WaitForEvent (1, &gST->ConIn->WaitForKey, &EventIndex);
      Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
      if (!EFI_ERROR(Status)) {
        switch (Key.UnicodeChar)
        {
        case 'q':
          SnakeStatus = SNAKE_GAME_OVER;
          break;
        case 'e':
          NewGame = TRUE; 
          break;
        default:
          break;
        }
      }
      if (NewGame) {
        break;
      }
    }
    InitSnake (&BltBufferInfo, &SnakeInfo);
    PaintGround (&BltBufferInfo, &BackPixel);
    PaintBorder(&BltBufferInfo, BltBufferInfo.GameStartX - 1, BltBufferInfo.GameStartY - 1, BltBufferInfo.GameSizeX + 1, BltBufferInfo.GameSizeY + 1,  &BoardPixel);
    PaintGameGround (&BltBufferInfo, &GameBackPixel);
    PaintSnake (&BltBufferInfo, &SnakeInfo);
    PaintStringOnScreen (&BltBufferInfo, &SnakeInfo);
    Flush(&BltBufferInfo);
    SnakeRun(&BltBufferInfo, &SnakeInfo, &SnakeStatus);
  }
}