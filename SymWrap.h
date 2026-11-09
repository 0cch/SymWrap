#ifndef SYMWRAP_H
#define SYMWRAP_H

#include <atlbase.h>
#include <atlstr.h>
#include <Dia2.h>

// #define DEBUG_PRINT

#ifdef _DEBUG
#define VERIFY_EXP(x)	ATLASSERT(x)		 
#else
#define VERIFY_EXP(x)	((void)(x))
#endif


#define HR_OK(x) SUCCEEDED(x)
#define HR_NOK(x) FAILED(x)
#define BOOL_STR(x) ((x) ? L"TRUE" : L"FALSE")


enum{
	SYMBOL_CHILDREN,
	SYMBOL_ACCESS,
	SYMBOL_ADDRESSOFFSET,
	SYMBOL_ADDRESSSECTION,
	SYMBOL_ADDRESSTAKEN,
	SYMBOL_AGE,
	SYMBOL_ARRAYINDEXTYPE,
	SYMBOL_ARRAYINDEXTYPEID,
	SYMBOL_BACKENDMAJOR,				// Not use
	SYMBOL_BACKENDMINOR,				// Not use
	SYMBOL_BACKENDBUILD,				// Not use
	SYMBOL_BASETYPE,
	SYMBOL_BITPOSITION,
	SYMBOL_CALLINGCONVENTION,
	SYMBOL_CLASSPARENT,
	SYMBOL_CLASSPARENTID,
	SYMBOL_CODE,
	SYMBOL_COMPILERGENERATED,
	SYMBOL_COMPILERNAME,
	SYMBOL_CONSTRUCTOR,
	SYMBOL_CONTAINER,
	SYMBOL_CONSTTYPE,
	SYMBOL_COUNT,
	SYMBOL_CUSTOMCALLINGCONVENTION,
	SYMBOL_DATABYTES,					// Not use
	SYMBOL_DATAKIND,
	SYMBOL_EDITANDCONTINUEENABLED,
	SYMBOL_FARRETURN,
	SYMBOL_FRONTENDMAJOR,				// Not use
	SYMBOL_FRONTENDMINOR,				// Not use
	SYMBOL_FRONTENDBUILD,				// Not use
	SYMBOL_FUNCTION,
	SYMBOL_GUID,
	SYMBOL_HASALLOCA,
	SYMBOL_HASASSIGNMENTOPERATOR,
	SYMBOL_HASCASTOPERATOR,
	SYMBOL_HASDEBUGINFO,
	SYMBOL_HASEH,
	SYMBOL_HASEHA,
	SYMBOL_HASINLASM,
	SYMBOL_HASLONGJUMP,
	SYMBOL_HASMANAGEDCODE,
	SYMBOL_HASNESTEDTYPES,
	SYMBOL_HASSECURITYCHECKS,
	SYMBOL_HASSEH,
	SYMBOL_HASSETJUMP,
	SYMBOL_INDIRECTVIRTUALBASECLASS,
	SYMBOL_INLSPEC,
	SYMBOL_INTERRUPTRETURN,
	SYMBOL_INTRO,
	SYMBOL_ISAGGREGATED,
	SYMBOL_ISCTYPES,
	SYMBOL_ISCVTCIL,
	SYMBOL_ISDATAALIGNED,
	SYMBOL_ISHOTPATCHABLE,
	SYMBOL_ISLTCG,
	SYMBOL_ISMSILNETMODULE,
	SYMBOL_ISNAKED,
	SYMBOL_ISSPLITTED,
	SYMBOL_ISSTATIC,
	SYMBOL_ISSTRIPPED,
	SYMBOL_LANGUAGE,
	SYMBOL_LENGTH,
	SYMBOL_LEXICALPARENT,
	SYMBOL_LEXICALPARENTID,
	SYMBOL_LIBRARYNAME,
	SYMBOL_LIVELVARINSTANCES,			// Not use
	SYMBOL_LOCATIONTYPE,
	SYMBOL_LOWERBOUND,					// Not use
	SYMBOL_LOWERBOUNDID,				// Not use
	SYMBOL_MACHINETYPE,
	SYMBOL_MANAGED,
	SYMBOL_MSIL,
	SYMBOL_NAME,
	SYMBOL_NESTED,
	SYMBOL_NOINLINE,
	SYMBOL_NORETURN,
	SYMBOL_NOSTACKORDERING,
	SYMBOL_NOTREACHED,
	SYMBOL_OBJECTPOINTERTYPE,
	SYMBOL_OEMID,
	SYMBOL_OEMSYMBOLID,
	SYMBOL_OFFSET,
	SYMBOL_OPTIMIZEDCODEDEBUGINFO,
	SYMBOL_OVERLOADEDOPERATOR,
	SYMBOL_PACKED,
	SYMBOL_PLATFORM,
	SYMBOL_PURE,
	SYMBOL_RANK,						// Not use
	SYMBOL_REFERENCE,
	SYMBOL_REGISTERID,
	SYMBOL_RELATIVEVIRTUALADDRESS,
	SYMBOL_SCOPED,
	SYMBOL_SIGNATURE,
	SYMBOL_SLOT,
	SYMBOL_SOURCEFILENAME,
	SYMBOL_SYMBOLSFILENAME,
	SYMBOL_SYMINDEXID,
	SYMBOL_SYMTAG,
	SYMBOL_TARGETOFFSET,
	SYMBOL_TARGETRELATIVEVIRTUALADDRESS,
	SYMBOL_TARGETSECTION,
	SYMBOL_TARGETVIRTUALADDRESS,
	SYMBOL_THISADJUST,
	SYMBOL_THUNKORDINAL,
	SYMBOL_TIMESTAMP,
	SYMBOL_TOKEN,
	SYMBOL_TYPE,
	SYMBOL_TYPEID,
	SYMBOL_TYPES,						// Not use
	SYMBOL_TYPEIDS,
	SYMBOL_UDTKIND,
	SYMBOL_UNALIGNEDTYPE,
	SYMBOL_UNDECORATEDNAME,
	SYMBOL_UNDECORATEDNAMEEX,			// Not use
	SYMBOL_UPPERBOUND,					// Not use
	SYMBOL_UPPERBOUNDID,				// Not use
	SYMBOL_VALUE,
	SYMBOL_VIRTUAL,
	SYMBOL_VIRTUALADDRESS,
	SYMBOL_VIRTUALBASECLASS,
	SYMBOL_VIRTUALBASEDISPINDEX,
	SYMBOL_VIRTUALBASEOFFSET,
	SYMBOL_VIRTUALBASEPOINTEROFFSET,
	SYMBOL_VIRTUALBASETABLETYPE,
	SYMBOL_VIRTUALTABLESHAPE,
	SYMBOL_VIRTUALTABLESHAPEID,
	SYMBOL_VOLATILETYPE,
};


typedef CSimpleMap<ULONG, CString> SymbolInfoTable;


class SymLoader {
private:
	class ComInitializer {
	public:
		ComInitializer();
		~ComInitializer();

		ComInitializer(const ComInitializer &) = delete;
		ComInitializer &operator=(const ComInitializer &) = delete;

		HRESULT Result() const { return _Result; }
		bool IsUsable() const { return SUCCEEDED(_Result) || _Result == RPC_E_CHANGED_MODE; }

	private:
		HRESULT _Result;
		bool _OwnsInitialization;
	};

	ComInitializer _ComInitializer;
	CComPtr<IDiaDataSource> _DataSource;
	CComPtr<IDiaSession> _DiaSession;

public:
	SymLoader();
	~SymLoader() = default;

	SymLoader(const SymLoader &) = delete;
	SymLoader &operator=(const SymLoader &) = delete;

	HRESULT Init();
	HRESULT Open(LPCWSTR FilePath);
	HRESULT GetSymbol(CComPtr<IDiaSymbol> &Symbol);
	// Compatibility overload. The returned interface has one owned reference.
	IDiaSymbol *GetSymbol();
};

class SymEnumTool {

public:
	// The raw-pointer constructor adds its own reference; the caller still owns
	// the reference passed in. Prefer the CComPtr overload.
	explicit SymEnumTool(IDiaEnumSymbols *DiaEnumSymbols) : _DiaEnumSymbols(DiaEnumSymbols) {}
	explicit SymEnumTool(const CComPtr<IDiaEnumSymbols> &DiaEnumSymbols) : _DiaEnumSymbols(DiaEnumSymbols) {}
	~SymEnumTool() {}

	HRESULT GetCount(LONG &Count) const;
	LONG GetCount() const;
	HRESULT Item(ULONG Index, CComPtr<IDiaSymbol> &Symbol) const;
	// Compatibility overloads return one owned reference.
	IDiaSymbol* Item(ULONG Index);
	HRESULT Next(CComPtr<IDiaSymbol> &Symbol) const;
	IDiaSymbol* Next();
	HRESULT Skip(ULONG Celt) const;
	HRESULT Reset() const;

private:
	CComPtr<IDiaEnumSymbols> _DiaEnumSymbols;
};

class SymBase {

public:
	// The raw-pointer constructor adds its own reference; the caller still owns
	// the reference passed in. Prefer the CComPtr overload.
	explicit SymBase(IDiaSymbol *DiaSymbol) : _DiaSymbol(DiaSymbol) {}
	explicit SymBase(const CComPtr<IDiaSymbol> &DiaSymbol) : _DiaSymbol(DiaSymbol) {}
	virtual ~SymBase() = default;

	HRESULT GetEnum(CComPtr<IDiaEnumSymbols> &Result, enum SymTagEnum Symtag = SymTagNull, LPCWSTR Name = NULL, ULONG Flags = nsNone) const;
	// Compatibility overload. The returned enumerator has one owned reference.
	IDiaEnumSymbols* GetEnum(enum SymTagEnum Symtag = SymTagNull, LPCWSTR Name = NULL, ULONG Flags = nsNone) const;
	HRESULT GetTag(ULONG &SymTag) const;
	ULONG GetTag() const;
	const CComPtr<IDiaSymbol> &Self() const {return _DiaSymbol;}
	virtual HRESULT GetDecl(CString &Info);
	virtual HRESULT GetDef(CString &Info);
	HRESULT GetInfoTable(SymbolInfoTable &InfoTable);
	void DisplayInfo(SymbolInfoTable &InfoTable);
	void EnumEveryChild();
	


	// The raw-pointer overload adds its own reference; the caller retains the
	// supplied reference. Prefer the CComPtr overload.
	static SymBase* SymNew(IDiaSymbol *Type);
	static SymBase* SymNew(const CComPtr<IDiaSymbol> &Type);
	

protected:
	LPCWSTR GetBaseTypeStr(ULONG Index, ULONG Length) const;
	LPCWSTR GetAccessStr(ULONG Index) const;
	LPCWSTR GetLanguageStr(ULONG Index) const;
	LPCWSTR GetLocationTypeStr(ULONG Index) const;
	LPCWSTR GetMachineTypeStr(ULONG Index) const;
	LPCWSTR GetSymTagStr(ULONG Index) const;
	LPCWSTR GetUdtKindStr(ULONG Index) const;
	LPCWSTR GetSymbolTypeStr(ULONG Index) const;
	LPCWSTR GetCallingConventionStr(ULONG Index) const;
	LPCWSTR GetDataKindStr(ULONG Index) const;

private:
	CComPtr<IDiaSymbol> _DiaSymbol;
};

class SymFunc : public SymBase {

public:
	SymFunc(IDiaSymbol *DiaSymbol) : SymBase(DiaSymbol) {}
	~SymFunc() {}

	virtual HRESULT GetDecl(CString &Info);
	virtual HRESULT GetDef(CString &Info);

};

class SymFuncType : public SymBase {

public:
	SymFuncType(IDiaSymbol *DiaSymbol) : SymBase(DiaSymbol) {}
	~SymFuncType() {}

	virtual HRESULT GetDecl(CString &Info);
	virtual HRESULT GetDef(CString &Info);
};

class SymBaseType : public SymBase {

public:
	SymBaseType(IDiaSymbol *DiaSymbol) : SymBase(DiaSymbol) {}
	~SymBaseType() {}

	virtual HRESULT GetDecl(CString &Info);
	virtual HRESULT GetDef(CString &Info);

private:
	LPCWSTR GetBaseTypeStr(const CComPtr<IDiaSymbol> &BaseType) const;
	
};


class SymFuncArgType : public SymBase {

public:
	SymFuncArgType(IDiaSymbol *DiaSymbol) : SymBase(DiaSymbol) {}
	~SymFuncArgType() {}

	virtual HRESULT GetDecl(CString &Info);
	virtual HRESULT GetDef(CString &Info);
};


class SymPointerType : public SymBase {

public:
	SymPointerType(IDiaSymbol *DiaSymbol) : SymBase(DiaSymbol) {}
	~SymPointerType() {}

	virtual HRESULT GetDecl(CString &Info);
	virtual HRESULT GetDef(CString &Info);
};


class SymUDT : public SymBase {

public:
	SymUDT(IDiaSymbol *DiaSymbol) : SymBase(DiaSymbol) {}
	~SymUDT() {}

	virtual HRESULT GetDecl(CString &Info);
	virtual HRESULT GetDef(CString &Info);
};


class SymEnum : public SymBase {

public:
	SymEnum(IDiaSymbol *DiaSymbol) : SymBase(DiaSymbol) {}
	~SymEnum() {}

	virtual HRESULT GetDecl(CString &Info);
	virtual HRESULT GetDef(CString &Info);
};


class SymDataType : public SymBase {

public:
	SymDataType(IDiaSymbol *DiaSymbol) : SymBase(DiaSymbol) {}
	~SymDataType() {}

	virtual HRESULT GetDecl(CString &Info);
	virtual HRESULT GetDef(CString &Info);
};

class SymTypedef : public SymBase {

public:
	SymTypedef(IDiaSymbol *DiaSymbol) : SymBase(DiaSymbol) {}
	~SymTypedef() {}

	virtual HRESULT GetDecl(CString &Info);
	virtual HRESULT GetDef(CString &Info);
};


class SymArray : public SymBase {

public:
	SymArray(IDiaSymbol *DiaSymbol) : SymBase(DiaSymbol) {}
	~SymArray() {}

	virtual HRESULT GetDecl(CString &Info);
	virtual HRESULT GetDef(CString &Info);
};


#endif