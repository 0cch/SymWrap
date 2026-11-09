SymWrap
=======

A C++ wrapper for the Debug Interface Access (DIA) SDK.

### Requirements

- Build with `UNICODE` and `_UNICODE` defined.
- Add the DIA SDK include directory to the include path and link `diaguids.lib`,
  `ole32.lib`, and `oleaut32.lib`.
- Deploy `msdia140.dll` next to the executable or next to the DLL that contains
  `SymWrap`. `SymLoader::Init()` first tries the registered DIA COM server and
  then uses registration-free loading from those locations.

### Sample

```cpp
#include "SymWrap.h"
#include <cwchar>

int wmain()
{
    const CString PdbName = L"sample.pdb";
    const CString Regexp = L"_PEB";
    const enum SymTagEnum Type = SymTagUDT; // or SymTagFunction

    SymLoader Loader;
    HRESULT hr = Loader.Init();
    if (FAILED(hr)) {
        wprintf(L"Initialize loader failed: 0x%08X\n", static_cast<unsigned>(hr));
        return 1;
    }

    hr = Loader.Open(PdbName.GetString());
    if (FAILED(hr)) {
        wprintf(L"Open PDB failed: 0x%08X\n", static_cast<unsigned>(hr));
        return 2;
    }

    CComPtr<IDiaSymbol> BaseSymbol;
    hr = Loader.GetSymbol(BaseSymbol);
    if (FAILED(hr) || BaseSymbol == NULL) {
        wprintf(L"Get base symbol failed: 0x%08X\n", static_cast<unsigned>(hr));
        return 3;
    }

    SymBase Base(BaseSymbol);
    CComPtr<IDiaEnumSymbols> EnumSymbols;
    hr = Base.GetEnum(EnumSymbols, Type, Regexp.GetString(),
        nsCaseInRegularExpression);
    if (FAILED(hr) || EnumSymbols == NULL) {
        wprintf(L"Enumerate symbols failed: 0x%08X\n", static_cast<unsigned>(hr));
        return 4;
    }

    SymEnumTool Enumer(EnumSymbols);
    LONG Count = 0;
    hr = Enumer.GetCount(Count);
    if (FAILED(hr)) {
        return 5;
    }

    for (LONG i = 0; i < Count; ++i) {
        CComPtr<IDiaSymbol> Symbol;
        hr = Enumer.Item(static_cast<ULONG>(i), Symbol);
        if (FAILED(hr) || Symbol == NULL) {
            continue;
        }

        CAutoPtr<SymBase> p(SymBase::SymNew(Symbol));
        if (p == NULL) {
            continue;
        }

        CString Text;
        hr = p->GetDef(Text);
        if (SUCCEEDED(hr)) {
            wprintf(L"%s\n", Text.GetString());
        }
    }

    return 0;
}
```