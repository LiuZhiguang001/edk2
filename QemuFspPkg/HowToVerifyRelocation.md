# How To Verify Relocation entries

SecCores of FSP M and FSP S can rebase themselves by calling PeCoffLib to rebase in very early phase. This assumes the PeCoffLib functions do not need to be rebased. To verify that, need to verify that relocation entries in the PeImage doesn't point to PeCoffLib functions code section. Below is the steps.

1. Use IntelFsp2Pkg\Tools\PeDump.py to dump the relocation entries of the Fsp SecCores. For example: \
```py PeDump.py <BuildFolder>\Fsp24SecCoreS\DEBUG\Fsp24SecCoreS.dll```
2. Check the offset in the dump result, and compare with the *Rva+Base* in map file (for example Fsp24SecCoreS.map)
3. Make sure the offset doesn't point to PeCoffLib functions code section.