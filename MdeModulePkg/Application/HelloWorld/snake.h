#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PciLib.h>
#include <Library/IoLib.h>
#include <Library/BaseCryptLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/HiiImage.h>
#include <Protocol/HiiFont.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
typedef enum {
  BOARD_UP,
  BOARD_DOWN,
  BOARD_RIGHT,
  BOARD_LEFT
} BOARD;

typedef enum {
  SNAKE_UP,
  SNAKE_DOWN,
  SNAKE_RIGHT,
  SNAKE_LEFT
} SNAKE_ORIENTATION;

typedef enum {
  SNAKE_GOOD,
  SNAKE_HIT_BOARD,
  SNAKE_HIT_ITSELF,
  SNAKE_GAME_OVER
} SNAKE_STATUS;

typedef struct {
  EFI_GRAPHICS_OUTPUT_PROTOCOL          *GraphicsInterface;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL         *BltBuffer;
  UINTN                                 GroundXSize;
  UINTN                                 GroundYSize;
  UINTN                                 BlockdXSize;
  UINTN                                 BlockdYSize;
  UINTN                                 GameSizeX;
  UINTN                                 GameSizeY;
  UINTN                                 GameStartX;
  UINTN                                 GameStartY;
} BLT_BUFFER_INFO;

typedef struct {
  UINTN                       X;
  UINTN                       Y;
} SNAKE_POINT;

typedef struct {
  SNAKE_POINT                           *SnakeArry;
  UINTN                                 SnakeLength;
  SNAKE_ORIENTATION                     SnakeOrientaion;
  SNAKE_ORIENTATION                     SnakeBackOrientaion;
} SNAKE_INFO;



VOID
SnakeMain (
  VOID
  );