## @ SplitFspBin.py
#
# Copyright (c) 2015 - 2022, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

import os
import sys
import uuid
import copy
import struct
import argparse
from   ctypes import *
from functools import reduce

"""
This utility supports to dump informaiton from a PE image.
It supports:
    - Dump relocation information
"""

class EFI_IMAGE_DATA_DIRECTORY(Structure):
    _fields_ = [
        ('VirtualAddress',       c_uint32),
        ('Size',                 c_uint32)
        ]

class EFI_IMAGE_DOS_HEADER(Structure):
    _fields_ = [
        ('e_magic',              c_uint16),
        ('e_cblp',               c_uint16),
        ('e_cp',                 c_uint16),
        ('e_crlc',               c_uint16),
        ('e_cparhdr',            c_uint16),
        ('e_minalloc',           c_uint16),
        ('e_maxalloc',           c_uint16),
        ('e_ss',                 c_uint16),
        ('e_sp',                 c_uint16),
        ('e_csum',               c_uint16),
        ('e_ip',                 c_uint16),
        ('e_cs',                 c_uint16),
        ('e_lfarlc',             c_uint16),
        ('e_ovno',               c_uint16),
        ('e_res',                ARRAY(c_uint16, 4)),
        ('e_oemid',              c_uint16),
        ('e_oeminfo',            c_uint16),
        ('e_res2',               ARRAY(c_uint16, 10)),
        ('e_lfanew',             c_uint16)
        ]

class EFI_IMAGE_FILE_HEADER(Structure):
    _fields_ = [
        ('Machine',               c_uint16),
        ('NumberOfSections',      c_uint16),
        ('TimeDateStamp',         c_uint32),
        ('PointerToSymbolTable',  c_uint32),
        ('NumberOfSymbols',       c_uint32),
        ('SizeOfOptionalHeader',  c_uint16),
        ('Characteristics',       c_uint16)
        ]

class PE_RELOC_BLOCK_HEADER(Structure):
    _fields_ = [
        ('PageRVA',              c_uint32),
        ('BlockSize',            c_uint32)
        ]

class EFI_IMAGE_OPTIONAL_HEADER32(Structure):
    _fields_ = [
        ('Magic',                         c_uint16),
        ('MajorLinkerVersion',            c_uint8),
        ('MinorLinkerVersion',            c_uint8),
        ('SizeOfCode',                    c_uint32),
        ('SizeOfInitializedData',         c_uint32),
        ('SizeOfUninitializedData',       c_uint32),
        ('AddressOfEntryPoint',           c_uint32),
        ('BaseOfCode',                    c_uint32),
        ('BaseOfData',                    c_uint32),
        ('ImageBase',                     c_uint32),
        ('SectionAlignment',              c_uint32),
        ('FileAlignment',                 c_uint32),
        ('MajorOperatingSystemVersion',   c_uint16),
        ('MinorOperatingSystemVersion',   c_uint16),
        ('MajorImageVersion',             c_uint16),
        ('MinorImageVersion',             c_uint16),
        ('MajorSubsystemVersion',         c_uint16),
        ('MinorSubsystemVersion',         c_uint16),
        ('Win32VersionValue',             c_uint32),
        ('SizeOfImage',                   c_uint32),
        ('SizeOfHeaders',                 c_uint32),
        ('CheckSum'     ,                 c_uint32),
        ('Subsystem',                     c_uint16),
        ('DllCharacteristics',            c_uint16),
        ('SizeOfStackReserve',            c_uint32),
        ('SizeOfStackCommit' ,            c_uint32),
        ('SizeOfHeapReserve',             c_uint32),
        ('SizeOfHeapCommit' ,             c_uint32),
        ('LoaderFlags'     ,              c_uint32),
        ('NumberOfRvaAndSizes',           c_uint32),
        ('DataDirectory',                 ARRAY(EFI_IMAGE_DATA_DIRECTORY, 16))
        ]

class EFI_IMAGE_OPTIONAL_HEADER32_PLUS(Structure):
    _fields_ = [
        ('Magic',                         c_uint16),
        ('MajorLinkerVersion',            c_uint8),
        ('MinorLinkerVersion',            c_uint8),
        ('SizeOfCode',                    c_uint32),
        ('SizeOfInitializedData',         c_uint32),
        ('SizeOfUninitializedData',       c_uint32),
        ('AddressOfEntryPoint',           c_uint32),
        ('BaseOfCode',                    c_uint32),
        ('ImageBase',                     c_uint64),
        ('SectionAlignment',              c_uint32),
        ('FileAlignment',                 c_uint32),
        ('MajorOperatingSystemVersion',   c_uint16),
        ('MinorOperatingSystemVersion',   c_uint16),
        ('MajorImageVersion',             c_uint16),
        ('MinorImageVersion',             c_uint16),
        ('MajorSubsystemVersion',         c_uint16),
        ('MinorSubsystemVersion',         c_uint16),
        ('Win32VersionValue',             c_uint32),
        ('SizeOfImage',                   c_uint32),
        ('SizeOfHeaders',                 c_uint32),
        ('CheckSum'     ,                 c_uint32),
        ('Subsystem',                     c_uint16),
        ('DllCharacteristics',            c_uint16),
        ('SizeOfStackReserve',            c_uint64),
        ('SizeOfStackCommit' ,            c_uint64),
        ('SizeOfHeapReserve',             c_uint64),
        ('SizeOfHeapCommit' ,             c_uint64),
        ('LoaderFlags'     ,              c_uint32),
        ('NumberOfRvaAndSizes',           c_uint32),
        ('DataDirectory',                 ARRAY(EFI_IMAGE_DATA_DIRECTORY, 16))
        ]

class EFI_IMAGE_OPTIONAL_HEADER(Union):
    _fields_ = [
        ('PeOptHdr',             EFI_IMAGE_OPTIONAL_HEADER32),
        ('PePlusOptHdr',         EFI_IMAGE_OPTIONAL_HEADER32_PLUS)
        ]

class EFI_IMAGE_NT_HEADERS32(Structure):
    _fields_ = [
        ('Signature',            c_uint32),
        ('FileHeader',           EFI_IMAGE_FILE_HEADER),
        ('OptionalHeader',       EFI_IMAGE_OPTIONAL_HEADER)
        ]


class EFI_IMAGE_DIRECTORY_ENTRY:
    EXPORT                     = 0
    IMPORT                     = 1
    RESOURCE                   = 2
    EXCEPTION                  = 3
    SECURITY                   = 4
    BASERELOC                  = 5
    DEBUG                      = 6
    COPYRIGHT                  = 7
    GLOBALPTR                  = 8
    TLS                        = 9
    LOAD_CONFIG                = 10

def AlignPtr (offset, alignment = 8):
    return (offset + alignment - 1) & ~(alignment - 1)


class PeTeImage:
    def __init__(self, path):
        f = open (path, 'rb')
        data = bytearray(f.read())
        f.close()
        self.DosHdr  = EFI_IMAGE_DOS_HEADER.from_buffer (data, 0)
        self.PeHdr   = EFI_IMAGE_NT_HEADERS32.from_buffer (data, self.DosHdr.e_lfanew)
        if self.PeHdr.Signature != 0x4550:
            raise Exception("ERROR: Invalid PE32 header !")
        if self.PeHdr.OptionalHeader.PeOptHdr.Magic == 0x10b: # PE32 image
            if self.PeHdr.FileHeader.SizeOfOptionalHeader < EFI_IMAGE_OPTIONAL_HEADER32.DataDirectory.offset:
                raise Exception("ERROR: Unsupported PE32 image !")
            if self.PeHdr.OptionalHeader.PeOptHdr.NumberOfRvaAndSizes <= EFI_IMAGE_DIRECTORY_ENTRY.BASERELOC:
                raise Exception("ERROR: No relocation information available !")
        elif self.PeHdr.OptionalHeader.PeOptHdr.Magic == 0x20b: # PE32+ image
            if self.PeHdr.FileHeader.SizeOfOptionalHeader < EFI_IMAGE_OPTIONAL_HEADER32_PLUS.DataDirectory.offset:
                raise Exception("ERROR: Unsupported PE32+ image !")
            if self.PeHdr.OptionalHeader.PePlusOptHdr.NumberOfRvaAndSizes <= EFI_IMAGE_DIRECTORY_ENTRY.BASERELOC:
                raise Exception("ERROR: No relocation information available !")
        else:
            raise Exception("ERROR: Invalid PE32 optional header !")

        self.Data      = data
        self.RelocList = []


    def ParseReloc(self):
        # Assuming PE32 image type (self.PeHdr.OptionalHeader.PeOptHdr.Magic == 0x10b)
        rsize   = self.PeHdr.OptionalHeader.PeOptHdr.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY.BASERELOC].Size
        roffset = self.PeHdr.OptionalHeader.PeOptHdr.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY.BASERELOC].VirtualAddress
        if self.PeHdr.OptionalHeader.PePlusOptHdr.Magic == 0x20b: # PE32+ image
            rsize   = self.PeHdr.OptionalHeader.PePlusOptHdr.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY.BASERELOC].Size
            roffset = self.PeHdr.OptionalHeader.PePlusOptHdr.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY.BASERELOC].VirtualAddress

        offset = roffset
        while offset < roffset + rsize:
            offset = AlignPtr(offset, 4)
            blkhdr = PE_RELOC_BLOCK_HEADER.from_buffer(self.Data, offset)
            offset += sizeof(blkhdr)
            # Read relocation type,offset pairs
            rlen  = blkhdr.BlockSize - sizeof(PE_RELOC_BLOCK_HEADER)
            rnum  = int (rlen/sizeof(c_uint16))
            rdata = (c_uint16 * rnum).from_buffer(self.Data, offset)
            for each in rdata:
                roff  = each & 0xfff
                rtype = each >> 12
                if rtype == 0: # IMAGE_REL_BASED_ABSOLUTE:
                    continue
                if ((rtype != 3) and (rtype != 10)): # IMAGE_REL_BASED_HIGHLOW and IMAGE_REL_BASED_DIR64
                    raise Exception("ERROR: Unsupported relocation type %d!" % rtype)
                # Calculate the offset of the relocation
                aoff  = blkhdr.PageRVA + roff
                self.RelocList.append((rtype, aoff))
            offset += sizeof(rdata)
    
    def DumpRelocationInfo(self):
        self.ParseReloc()
        if len(self.RelocList) == 0:
            print("No relocation entries in this PE image")
        else:
            print("Offset\trelaction type")
            for relocEntry in self.RelocList:
                print(f"{relocEntry[1]}\t{relocEntry[0]}")

def main ():
    parser = argparse.ArgumentParser(description='Dump PE image')
    parser.add_argument("PeImage", action="store", default="", help="The Image file need to be dumpped")
    args = parser.parse_args()
    
    imagePath = args.PeImage
    if not os.path.isabs(imagePath):
        imagePath = os.path.join(os.path.curdir, imagePath)
    if os.path.exists(imagePath) and os.path.isfile(imagePath):
        print(f"Going to dump informtion for {imagePath}")
        pe = PeTeImage(imagePath)
        pe.DumpRelocationInfo()
    else:
        print("Can not find Pe Image. Please check the path below")
        print(imagePath)
    return 0

if __name__ == '__main__':
    sys.exit(main())
