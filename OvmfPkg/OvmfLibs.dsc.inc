
[LibraryClasses.common.PEIM]
  MemEncryptSevLib|OvmfPkg\Library\BaseMemEncryptSevLib\BaseMemEncryptSevLib.inf

[LibraryClasses.common.DXE_DRIVER]
  MemEncryptSevLib|OvmfPkg\Library\BaseMemEncryptSevLib\BaseMemEncryptSevLib.inf
  PlatformBmPrintScLib|OvmfPkg\Library\PlatformBmPrintScLib\PlatformBmPrintScLib.inf
  PlatformHasIoMmuLib|OvmfPkg\Library\PlatformHasIoMmuLib\PlatformHasIoMmuLib.inf
  QemuBootOrderLib|OvmfPkg\Library\QemuBootOrderLib\QemuBootOrderLib.inf
  SmbiosVersionLib|OvmfPkg\Library\SmbiosVersionLib\DetectSmbiosVersionLib.inf
  TlsAuthConfigLib|OvmfPkg\Library\TlsAuthConfigLib\TlsAuthConfigLib.inf

[LibraryClasses.common.DXE_RUNTIME_DRIVER]
  MemEncryptSevLib|OvmfPkg\Library\BaseMemEncryptSevLib\BaseMemEncryptSevLib.inf

[LibraryClasses.common.DXE_SMM_DRIVER]
  MemEncryptSevLib|OvmfPkg\Library\BaseMemEncryptSevLib\BaseMemEncryptSevLib.inf

[LibraryClasses.common.UEFI_DRIVER]
  MemEncryptSevLib|OvmfPkg\Library\BaseMemEncryptSevLib\BaseMemEncryptSevLib.inf

[LibraryClasses.common]
  CsmSupportLib|OvmfPkg\Csm\CsmSupportLib\CsmSupportLib.inf
  PciCapLib|OvmfPkg\Library\BasePciCapLib\BasePciCapLib.inf
  PciCapPciSegmentLib|OvmfPkg\Library\BasePciCapPciSegmentLib\BasePciCapPciSegmentLib.inf
  LoadLinuxLib|OvmfPkg\Library\LoadLinuxLib\LoadLinuxLib.inf
  QemuFwCfgSimpleParserLib|OvmfPkg\Library\QemuFwCfgSimpleParserLib\QemuFwCfgSimpleParserLib.inf
  PciCapPciIoLib|OvmfPkg\Library\UefiPciCapPciIoLib\UefiPciCapPciIoLib.inf
  VirtioLib|OvmfPkg\Library\VirtioLib\VirtioLib.inf
  VirtioMmioDeviceLib|OvmfPkg\Library\VirtioMmioDeviceLib\VirtioMmioDeviceLib.inf
  XenHypercallLib|OvmfPkg\Library\XenHypercallLib\XenHypercallLib.inf
  XenIoMmioLib|OvmfPkg\Library\XenIoMmioLib\XenIoMmioLib.inf
  XenPlatformLib|OvmfPkg\Library\XenPlatformLib\XenPlatformLib.inf

[LibraryClasses.X64.PEIM]
  BhyveFwCtlLib|OvmfPkg\Library\BhyveFwCtlLib\BhyveFwCtlLib.inf

[LibraryClasses.X64.DXE_DRIVER]
  BhyveFwCtlLib|OvmfPkg\Library\BhyveFwCtlLib\BhyveFwCtlLib.inf

[LibraryClasses.X64.DXE_RUNTIME_DRIVER]
  BhyveFwCtlLib|OvmfPkg\Library\BhyveFwCtlLib\BhyveFwCtlLib.inf
