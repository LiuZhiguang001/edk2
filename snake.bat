@if "%1" == "ovmf" (
  goto ovmf
)

call build -p OvmfPkg\OvmfPkgIa32X64.dsc -a IA32 -a X64 -m MdeModulePkg\Application\HelloWorld\HelloWorld.inf  -t CLANGPDB -D DEBUG_ON_SERIAL_PORT=TRUE
goto done

:ovmf
call build -p OvmfPkg\OvmfPkgIa32X64.dsc -a IA32 -a X64 -t CLANGPDB  -DDEBUG_ON_SERIAL_PORT -D DEBUG_ON_SERIAL_PORT=TRUE


:done
if not %ERRORLEVEL% == 0 exit /b 1
call C:\code\Payload\edk2_nanopb2\qemu\qemu-system-x86_64.exe  -hdb fat:rw:C:\code\snake\edk2\Build\Ovmf3264\DEBUG_CLANGPDB\X64  -machine q35 -drive file=Build\Ovmf3264\DEBUG_CLANGPDB\FV\OVMF.fd,if=pflash,format=raw -boot menu=on,splash-time=0 -usb -device nec-usb-xhci,id=xhci -device usb-kbd   -device usb-mouse -net none -serial mon:stdio
