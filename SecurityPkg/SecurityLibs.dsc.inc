
[LibraryClasses.common.PEIM]
  TpmCommLib|SecurityPkg\Library\TpmCommLib\TpmCommLib.inf

[LibraryClasses.common.DXE_DRIVER]
  TcgPhysicalPresenceLib|SecurityPkg\Library\DxeTcgPhysicalPresenceLib\DxeTcgPhysicalPresenceLib.inf
  Tcg2PpVendorLib|SecurityPkg\Library\Tcg2PpVendorLibNull\Tcg2PpVendorLibNull.inf
  TcgPpVendorLib|SecurityPkg\Library\TcgPpVendorLibNull\TcgPpVendorLibNull.inf
  TpmCommLib|SecurityPkg\Library\TpmCommLib\TpmCommLib.inf

[LibraryClasses.common.DXE_RUNTIME_DRIVER]
  TcgPhysicalPresenceLib|SecurityPkg\Library\DxeTcgPhysicalPresenceLib\DxeTcgPhysicalPresenceLib.inf
  Tcg2PpVendorLib|SecurityPkg\Library\Tcg2PpVendorLibNull\Tcg2PpVendorLibNull.inf
  TcgPpVendorLib|SecurityPkg\Library\TcgPpVendorLibNull\TcgPpVendorLibNull.inf

[LibraryClasses.common.DXE_SMM_DRIVER]
  Tcg2PpVendorLib|SecurityPkg\Library\Tcg2PpVendorLibNull\Tcg2PpVendorLibNull.inf
  TcgPpVendorLib|SecurityPkg\Library\TcgPpVendorLibNull\TcgPpVendorLibNull.inf
  TpmCommLib|SecurityPkg\Library\TpmCommLib\TpmCommLib.inf

[LibraryClasses.common.UEFI_DRIVER]
  TcgPhysicalPresenceLib|SecurityPkg\Library\DxeTcgPhysicalPresenceLib\DxeTcgPhysicalPresenceLib.inf
  TpmCommLib|SecurityPkg\Library\TpmCommLib\TpmCommLib.inf

[LibraryClasses.common]
  RpmcLib|SecurityPkg\Library\RpmcLibNull\RpmcLibNull.inf
  TcgStorageCoreLib|SecurityPkg\Library\TcgStorageCoreLib\TcgStorageCoreLib.inf
  TcgStorageOpalLib|SecurityPkg\Library\TcgStorageOpalLib\TcgStorageOpalLib.inf
  Tpm12CommandLib|SecurityPkg\Library\Tpm12CommandLib\Tpm12CommandLib.inf
  Tpm2CommandLib|SecurityPkg\Library\Tpm2CommandLib\Tpm2CommandLib.inf
  VariableKeyLib|SecurityPkg\Library\VariableKeyLibNull\VariableKeyLibNull.inf

[LibraryClasses.common.UEFI_APPLICATION]
  TcgPhysicalPresenceLib|SecurityPkg\Library\DxeTcgPhysicalPresenceLib\DxeTcgPhysicalPresenceLib.inf
