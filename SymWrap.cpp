#include "SymWrap.h"


HRESULT SymLoader::Open( LPWSTR FilePath )
{
	HRESULT hr = _DataSource->loadDataFromPdb(FilePath);
	ATLASSERT(HR_OK(hr));
	return hr;
}

IDiaSymbol * SymLoader::GetSymbol()
{
	HRESULT hr = _DataSource->openSession(&_DiaSession);
	ATLASSERT(HR_OK(hr));
	if (HR_NOK(hr)) {
		return NULL;
	}

	IDiaSymbol *GlobalSymbol = NULL;
	hr = _DiaSession->get_globalScope(&GlobalSymbol);
	ATLASSERT(HR_OK(hr));

	return GlobalSymbol;
}

HRESULT SymLoader::Init()
{
	HRESULT hr = CoCreateInstance(CLSID_DiaSource,
		NULL,
		CLSCTX_INPROC_SERVER,
		__uuidof(IDiaDataSource),
		(PVOID *)&_DataSource);
	ATLASSERT(HR_OK(hr));
	if (HR_NOK(hr)) {

typedef HRESULT (__stdcall *DLLREGISTERSERVER)();

		HMODULE Moudle = LoadLibraryW(L"msdia100.dll");
		if (Moudle == NULL) {
			return hr;
		}

		DLLREGISTERSERVER RegisterServerFunc 
			= (DLLREGISTERSERVER)GetProcAddress(Moudle, "DllRegisterServer");
		if (RegisterServerFunc == NULL) {
			FreeLibrary(Moudle);
			return hr;
		}

		hr = RegisterServerFunc();
		FreeLibrary(Moudle);
		if (HR_NOK(hr)) {
			return hr;
		}

		hr = CoCreateInstance(CLSID_DiaSource,
			NULL,
			CLSCTX_INPROC_SERVER,
			__uuidof(IDiaDataSource),
			(PVOID *)&_DataSource);

	}

	return hr;
}

IDiaEnumSymbols* SymBase::GetEnum( enum SymTagEnum Symtag /*= SymTagNull*/, LPWSTR Name /*= NULL*/, ULONG Flags /*= nsNone*/ )
{
	IDiaEnumSymbols *DiaEnumSymbols = NULL;
	HRESULT hr = _DiaSymbol->findChildren(Symtag, 
		Name, 
		Flags, 
		&DiaEnumSymbols);

	ATLASSERT(HR_OK(hr));
	return DiaEnumSymbols;
}

ULONG SymBase::GetTag()
{
	ULONG SymTag = 0;
	HRESULT hr = _DiaSymbol->get_symTag(&SymTag);
	ATLASSERT(HR_OK(hr));
	return hr;
}

LPCWSTR SymBase::GetBaseTypeStr( ULONG Index, ULONG Length )
{
	LPCWSTR BaseTypeStr = NULL;

	switch (Index)
	{
	case btNoType   : 
		BaseTypeStr = L"NOTYPE";
		break;
	case btVoid     : 
		BaseTypeStr = L"VOID";
		break;
	case btChar     : 
		BaseTypeStr = L"CHAR";
		break;
	case btWChar    : 
		BaseTypeStr = L"WCHAR";
		break;
	case btInt      : 
		BaseTypeStr = L"INT";
		switch (Length)
		{
		case 1:
			BaseTypeStr = L"CHAR";
			break;
		case 2:
			BaseTypeStr = L"SHORT";
			break;
		case 4:
			BaseTypeStr = L"LONG";
			break;
		case 8:
			BaseTypeStr = L"LONGLONG";
			break;
		default:
			break;
		}
		break;
	case btUInt     : 
		BaseTypeStr = L"UINT";
		switch (Length)
		{
		case 1:
			BaseTypeStr = L"UCHAR";
			break;
		case 2:
			BaseTypeStr = L"USHORT";
			break;
		case 4:
			BaseTypeStr = L"ULONG";
			break;
		case 8:
			BaseTypeStr = L"ULONGLONG";
			break;
		default:
			break;
		}
		break;
	case btFloat    : 
		BaseTypeStr = L"FLOAT";
		switch (Length)
		{
		case 4:
			BaseTypeStr = L"FLOAT";
			break;
		case 8:
			BaseTypeStr = L"DOUBLE";
			break;
		default:
			break;
		}
		break;
	case btBCD      : 
		BaseTypeStr = L"BCD";
		break;
	case btBool     : 
		BaseTypeStr = L"BOOLEAN";
		break;
	case btLong     : 
		BaseTypeStr = L"LONG";
		break;
	case btULong    : 
		BaseTypeStr = L"ULONG";
		break;
	case btCurrency : 
		BaseTypeStr = L"CURRENCY";
		break;
	case btDate     : 
		BaseTypeStr = L"DATE";
		break;
	case btVariant  : 
		BaseTypeStr = L"VARIANT";
		break;
	case btComplex  : 
		BaseTypeStr = L"COMPLEX";
		break;
	case btBit      : 
		BaseTypeStr = L"BIT";
		break;
	case btBSTR     : 
		BaseTypeStr = L"BSTR";
		break;
	case btHresult  : 
		BaseTypeStr = L"HRESULT";
		break;
	default:
		break;
	}

	return BaseTypeStr;
}

LPCWSTR SymBase::GetAccessStr( ULONG Index )
{
	LPCWSTR AccessStr = NULL;

	switch (Index)
	{
	case CV_private:
		AccessStr = L"private";
		break;
	case CV_protected:
		AccessStr = L"protected";
		break;
	case CV_public:
		AccessStr = L"public";
		break;
	default:
		break;
	}

	return AccessStr;
}

SymBase* SymBase::SymNew( IDiaSymbol *Type )
{
	enum SymTagEnum Tag = SymTagNull;
	Type->get_symTag((PDWORD)&Tag);

	SymBase* Ret = NULL;
	switch (Tag)
	{
	case SymTagFunction:
		Ret = new SymFunc(Type);
		break;
	case SymTagFunctionType:
		Ret = new SymFuncType(Type);
		break;
	case SymTagBaseType:
		Ret = new SymBaseType(Type);
		break;
	case SymTagFunctionArgType:
		Ret = new SymFuncArgType(Type);
		break;
	case SymTagPointerType:
		Ret = new SymPointerType(Type);
		break;
	case SymTagUDT:
		Ret = new SymUDT(Type);
		break;
	case SymTagEnum:
		Ret = new SymEnum(Type);
		break;
	case SymTagData:
		Ret = new SymDataType(Type);
		break;
	case SymTagTypedef:
		Ret = new SymTypedef(Type);
		break;
	case SymTagArrayType:
		Ret = new SymArray(Type);
		break;

	default:
		Ret = new SymBase(Type);
		break;
	}
	return Ret;
}

HRESULT SymBase::GetInfoTable( SymbolInfoTable &InfoTable )
{
	HRESULT hr;
	ULONG SymULONG;
	LPCWSTR SymStr;
	BSTR SymBStr;
	BOOL SymBOOL;
	IDiaSymbol *SymSymbol;
	IDiaEnumSymbols *SymEnum;
	BOOL TableRet;
	CString Tmp;
	

	//
	// Children
	//
	hr = Self()->findChildren(SymTagNull, NULL, nsNone, &SymEnum);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_CHILDREN, BOOL_STR(SymEnum != NULL));
		VERIFY_EXP(TableRet);
		SymEnum->Release();
	}


	//
	// access
	//
	hr = Self()->get_access(&SymULONG);
	if (HR_OK(hr)) {
		SymStr = GetAccessStr(SymULONG);
		TableRet = InfoTable.Add(SYMBOL_ACCESS, SymStr);
		VERIFY_EXP(TableRet);
	}


	//
	// addressOffset
	//
	hr = Self()->get_addressOffset(&SymULONG);
	if (HR_OK(hr)) {
		Tmp.Format(L"%08X", SymULONG);
		TableRet = InfoTable.Add(SYMBOL_ADDRESSOFFSET, Tmp);
		VERIFY_EXP(TableRet);
	}


	//
	// addressSection 
	//
	hr = Self()->get_addressSection(&SymULONG);
	if (HR_OK(hr)) {
		Tmp.Format(L"%08X", SymULONG);
		TableRet = InfoTable.Add(SYMBOL_ADDRESSSECTION, Tmp);
		VERIFY_EXP(TableRet);
	}

	//
	// addressTaken 
	//
	hr = Self()->get_addressTaken(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_ADDRESSTAKEN, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}

	//
	// age 
	//
	hr = Self()->get_age(&SymULONG);
	if (HR_OK(hr)) {
		Tmp.Format(L"%08X", SymULONG);
		TableRet = InfoTable.Add(SYMBOL_AGE, Tmp);
		VERIFY_EXP(TableRet);
	}

	
	//
	// arrayIndexType
	//
	hr = Self()->get_arrayIndexType(&SymSymbol);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_ARRAYINDEXTYPE, BOOL_STR(SymSymbol != NULL));
		VERIFY_EXP(TableRet);
		SymSymbol->Release();
	}


	//
	// arrayIndexTypeId 
	//
	hr = Self()->get_arrayIndexTypeId(&SymULONG);
	if (HR_OK(hr)) {
		Tmp.Format(L"%08X", SymULONG);
		TableRet = InfoTable.Add(SYMBOL_ARRAYINDEXTYPEID, Tmp);
		VERIFY_EXP(TableRet);
	}


	//
	// backEndMajor (Not implement)
	//


	//
	// backEndMinor (Not implement)
	//


	//
	// backEndBuild (Not implement)
	//



	//
	// baseType 
	//
	hr = Self()->get_baseType(&SymULONG);
	if (HR_OK(hr)) {
		SymStr = GetBaseTypeStr(SymULONG, 0);
		TableRet = InfoTable.Add(SYMBOL_BASETYPE, SymStr);
		VERIFY_EXP(TableRet);
	}


	//
	// bitPosition 
	//
	hr = Self()->get_bitPosition(&SymULONG);
	if (HR_OK(hr)) {
		Tmp.Format(L"%08X", SymULONG);
		TableRet = InfoTable.Add(SYMBOL_BITPOSITION, Tmp);
		VERIFY_EXP(TableRet);
	}

	//
	// callingConvention 
	//
	hr = Self()->get_callingConvention(&SymULONG);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_CALLINGCONVENTION, GetCallingConventionStr(SymULONG));
		VERIFY_EXP(TableRet);
	}
	



	//
	// classParent
	//
	hr = Self()->get_classParent(&SymSymbol);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_CLASSPARENT, BOOL_STR(SymSymbol != NULL));
		VERIFY_EXP(TableRet);
		SymSymbol->Release();
	}


	//
	// classParentId 
	//
	hr = Self()->get_classParentId(&SymULONG);
	if (HR_OK(hr)) {
		Tmp.Format(L"%08X", SymULONG);
		TableRet = InfoTable.Add(SYMBOL_CLASSPARENTID, Tmp);
		VERIFY_EXP(TableRet);
	}

	
	//
	// code 
	//
	hr = Self()->get_code(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_CODE, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// compilerGenerated 
	//
	hr = Self()->get_compilerGenerated(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_COMPILERGENERATED, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}

	
	//
	// compilerName 
	//
	hr = Self()->get_compilerName(&SymBStr);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_COMPILERNAME, SymBStr);
		VERIFY_EXP(TableRet);
	}


	//
	// constructor 
	//
	hr = Self()->get_constructor(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_CONSTRUCTOR, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// container
	//
	hr = Self()->get_container(&SymSymbol);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_CONTAINER, BOOL_STR(SymSymbol != NULL));
		VERIFY_EXP(TableRet);
		SymSymbol->Release();
	}


	//
	// constType 
	//
	hr = Self()->get_constType(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_CONSTTYPE, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}



	//
	// count 
	//
	hr = Self()->get_count(&SymULONG);
	if (HR_OK(hr)) {
		Tmp.Format(L"%08X", SymULONG);
		TableRet = InfoTable.Add(SYMBOL_COUNT, Tmp);
		VERIFY_EXP(TableRet);
	}


	//
	// customCallingConvention
	//
	hr = Self()->get_customCallingConvention(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_CUSTOMCALLINGCONVENTION, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// dataBytes (Not implement)
	//


	//
	// dataKind 
	//
	hr = Self()->get_dataKind(&SymULONG);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_DATAKIND, GetDataKindStr(SymULONG));
		VERIFY_EXP(TableRet);
	}

	//
	// editAndContinueEnabled 
	//
	hr = Self()->get_editAndContinueEnabled(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_EDITANDCONTINUEENABLED, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}

	//
	// farReturn
	//
	hr = Self()->get_farReturn(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_FARRETURN, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// frontEndMajor (Not implement)
	//


	//
	// frontEndMinor (Not implement)
	//


	//
	// frontEndBuild (Not implement)
	//

	
	//
	// function 
	//
	hr = Self()->get_function(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_FARRETURN, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// guid 
	//
	GUID SymGuid;
	hr = Self()->get_guid(&SymGuid);
	if (HR_OK(hr)) {
		Tmp.Format(L"{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X} ", 
			SymGuid.Data1, SymGuid.Data2, SymGuid.Data3, SymGuid.Data4[0],
			SymGuid.Data4[1], SymGuid.Data4[2], SymGuid.Data4[3], 
			SymGuid.Data4[4], SymGuid.Data4[5], 
			SymGuid.Data4[6], SymGuid.Data4[7]);

		TableRet = InfoTable.Add(SYMBOL_GUID, Tmp);
		VERIFY_EXP(TableRet);
	}


	//
	// hasAlloca
	//
	hr = Self()->get_hasAlloca(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_HASALLOCA, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}



	//
	// hasAssignmentOperator 
	//
	hr = Self()->get_hasAssignmentOperator(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_HASASSIGNMENTOPERATOR, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// hasCastOperator 
	//
	hr = Self()->get_hasCastOperator(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_HASCASTOPERATOR, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// hasDebugInfo
	//
	hr = Self()->get_hasDebugInfo(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_HASDEBUGINFO, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// hasEH
	//
	hr = Self()->get_hasEH(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_HASEH, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// hasEHa
	//
	hr = Self()->get_hasEHa(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_HASEHA, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// hasInlAsm
	//
	hr = Self()->get_hasInlAsm(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_HASINLASM, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// hasLongJump
	//
	hr = Self()->get_hasLongJump(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_HASLONGJUMP, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// hasManagedCode
	//
	hr = Self()->get_hasManagedCode(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_HASMANAGEDCODE, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// hasNestedTypes 
	//
	hr = Self()->get_hasNestedTypes(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_HASNESTEDTYPES, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// hasSecurityChecks
	//
	hr = Self()->get_hasSecurityChecks(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_HASSECURITYCHECKS, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// hasSEH
	//
	hr = Self()->get_hasSEH(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_HASSEH, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// hasSetJump
	//
	hr = Self()->get_hasSetJump(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_HASSETJUMP, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// indirectVirtualBaseClass 
	//
	hr = Self()->get_indirectVirtualBaseClass(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_INDIRECTVIRTUALBASECLASS, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// inlSpec
	//
	hr = Self()->get_inlSpec(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_INLSPEC, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// interruptReturn
	//
	hr = Self()->get_interruptReturn(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_INTERRUPTRETURN, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// intro 
	//
	hr = Self()->get_intro(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_INTRO, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// isAggregated
	//
	hr = Self()->get_isAggregated(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_ISAGGREGATED, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// isCTypes
	//
	hr = Self()->get_isCTypes(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_ISCTYPES, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// isCVTCIL
	//
	hr = Self()->get_isCVTCIL(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_ISCVTCIL, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// isDataAligned
	//
	hr = Self()->get_isDataAligned(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_ISDATAALIGNED, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// isHotpatchable
	//
	hr = Self()->get_isHotpatchable(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_ISHOTPATCHABLE, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}

	//
	// isLTCG
	//
	hr = Self()->get_isLTCG(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_ISLTCG, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// isMSILNetmodule
	//
	hr = Self()->get_isMSILNetmodule(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_ISMSILNETMODULE, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// isNaked
	//
	hr = Self()->get_isNaked(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_ISNAKED, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// isSplitted
	//
	hr = Self()->get_isSplitted(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_ISSPLITTED, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// isStatic
	//
	hr = Self()->get_isStatic(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_ISSTATIC, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// isStripped
	//
	hr = Self()->get_isStripped(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_ISSTRIPPED, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// language 
	//
	hr = Self()->get_language(&SymULONG);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_LANGUAGE, GetLanguageStr(SymULONG));
		VERIFY_EXP(TableRet);
	}


	//
	// length 
	//
	ULONGLONG SymULONGLONG;
	hr = Self()->get_length(&SymULONGLONG);
	if (HR_OK(hr)) {
		Tmp.Format(L"%0.16I64X", SymULONGLONG);
		TableRet = InfoTable.Add(SYMBOL_LENGTH, Tmp);
		VERIFY_EXP(TableRet);
	}


	//
	// lexicalParent 
	//
	hr = Self()->get_lexicalParent(&SymSymbol);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_LEXICALPARENT, BOOL_STR(SymSymbol != NULL));
		VERIFY_EXP(TableRet);
		SymSymbol->Release();
	}

	//
	// lexicalParentId 
	//
	hr = Self()->get_lexicalParentId(&SymULONG);
	if (HR_OK(hr)) {
		Tmp.Format(L"%08X", SymULONG);
		TableRet = InfoTable.Add(SYMBOL_LEXICALPARENTID, Tmp);
		VERIFY_EXP(TableRet);
	}


	//
	// libraryName 
	//
	hr = Self()->get_libraryName(&SymBStr);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_LIBRARYNAME, SymBStr);
		VERIFY_EXP(TableRet);
	}


	//
	// liveLVarInstances (Not implement)
	//


	//
	// locationType 
	//
	hr = Self()->get_locationType(&SymULONG);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_LOCATIONTYPE, GetLocationTypeStr(SymULONG));
		VERIFY_EXP(TableRet);
	}


	//
	// lowerBound (Not implement)
	//


	//
	// lowerBoundId (Not implement)
	//


	//
	// machineType 
	//
	hr = Self()->get_machineType(&SymULONG);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_MACHINETYPE, GetMachineTypeStr(SymULONG));
		VERIFY_EXP(TableRet);
	}


	//
	// managed 
	//
	hr = Self()->get_managed(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_MANAGED, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}

	//
	// msil 
	//
	hr = Self()->get_msil(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_MSIL, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}

	//
	// name 
	//
	hr = Self()->get_name(&SymBStr);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_NAME, SymBStr);
		VERIFY_EXP(TableRet);
	}


	//
	// nested
	//
	hr = Self()->get_nested(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_NESTED, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// noInline
	//
	hr = Self()->get_noInline(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_NOINLINE, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// noReturn
	//
	hr = Self()->get_noReturn(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_NORETURN, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// noStackOrdering
	//
	hr = Self()->get_noStackOrdering(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_NOSTACKORDERING, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// notReached
	//
	hr = Self()->get_notReached(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_NOTREACHED, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// objectPointerType 
	//
	hr = Self()->get_objectPointerType(&SymSymbol);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_OBJECTPOINTERTYPE, BOOL_STR(SymSymbol != NULL));
		VERIFY_EXP(TableRet);
		SymSymbol->Release();
	}

	
	//
	// oemId 
	//
	hr = Self()->get_oemId(&SymULONG);
	if (HR_OK(hr)) {
		Tmp.Format(L"%08X", SymULONG);
		TableRet = InfoTable.Add(SYMBOL_OEMID, Tmp);
		VERIFY_EXP(TableRet);
	}


	//
	// oemSymbolId 
	//
	hr = Self()->get_oemSymbolId(&SymULONG);
	if (HR_OK(hr)) {
		Tmp.Format(L"%08X", SymULONG);
		TableRet = InfoTable.Add(SYMBOL_OEMSYMBOLID, Tmp);
		VERIFY_EXP(TableRet);
	}


	//
	// offset 
	//
	LONG SymLONG;
	hr = Self()->get_offset(&SymLONG);
	if (HR_OK(hr)) {
		Tmp.Format(L"%08X", SymLONG);
		TableRet = InfoTable.Add(SYMBOL_OFFSET, Tmp);
		VERIFY_EXP(TableRet);
	}


	//
	// optimizedCodeDebugInfo
	//
	hr = Self()->get_optimizedCodeDebugInfo(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_OPTIMIZEDCODEDEBUGINFO, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// overloadedOperator
	//
	hr = Self()->get_overloadedOperator(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_OVERLOADEDOPERATOR, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// packed
	//
	hr = Self()->get_packed(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_PACKED, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// platform 
	//
	hr = Self()->get_platform(&SymULONG);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_OEMSYMBOLID, GetMachineTypeStr(SymULONG));
		VERIFY_EXP(TableRet);
	}


	//
	// pure
	//
	hr = Self()->get_pure(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_PURE, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// rank (Not implement)
	//


	//
	// reference
	//
	hr = Self()->get_reference(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_REFERENCE, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// registerId 
	//
	hr = Self()->get_platform(&SymULONG);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_REGISTERID, GetMachineTypeStr(SymULONG));
		VERIFY_EXP(TableRet);
	}


	//
	// relativeVirtualAddress 
	//
	hr = Self()->get_platform(&SymULONG);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_RELATIVEVIRTUALADDRESS, GetMachineTypeStr(SymULONG));
		VERIFY_EXP(TableRet);
	}


	//
	// scoped 
	//
	hr = Self()->get_scoped(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_SCOPED, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// signature 
	//
	hr = Self()->get_signature(&SymULONG);
	if (HR_OK(hr)) {
		Tmp.Format(L"%08X", SymULONG);
		TableRet = InfoTable.Add(SYMBOL_SIGNATURE, Tmp);
		VERIFY_EXP(TableRet);
	}


	//
	// slot 
	//
	hr = Self()->get_slot(&SymULONG);
	if (HR_OK(hr)) {
		Tmp.Format(L"%08X", SymULONG);
		TableRet = InfoTable.Add(SYMBOL_SLOT, Tmp);
		VERIFY_EXP(TableRet);
	}


	//
	// sourceFileName 
	//
	hr = Self()->get_sourceFileName(&SymBStr);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_SOURCEFILENAME, SymBStr);
		VERIFY_EXP(TableRet);
	}


	//
	// symbolsFileName  
	//
	hr = Self()->get_symbolsFileName(&SymBStr);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_SYMBOLSFILENAME, SymBStr);
		VERIFY_EXP(TableRet);
	}


	//
	// symIndexId 
	//
	hr = Self()->get_symIndexId(&SymULONG);
	if (HR_OK(hr)) {
		Tmp.Format(L"%08X", SymULONG);
		TableRet = InfoTable.Add(SYMBOL_SYMINDEXID, Tmp);
		VERIFY_EXP(TableRet);
	}


	//
	// symTag 
	//
	hr = Self()->get_symTag(&SymULONG);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_SYMTAG, GetSymTagStr(SymULONG));
		VERIFY_EXP(TableRet);
	}


	//
	// targetOffset 
	//
	hr = Self()->get_targetOffset(&SymULONG);
	if (HR_OK(hr)) {
		Tmp.Format(L"%08X", SymULONG);
		TableRet = InfoTable.Add(SYMBOL_SYMINDEXID, Tmp);
		VERIFY_EXP(TableRet);
	}


	//
	// targetRelativeVirtualAddress 
	//
	hr = Self()->get_targetRelativeVirtualAddress(&SymULONG);
	if (HR_OK(hr)) {
		Tmp.Format(L"%08X", SymULONG);
		TableRet = InfoTable.Add(SYMBOL_TARGETRELATIVEVIRTUALADDRESS, Tmp);
		VERIFY_EXP(TableRet);
	}


	//
	// targetSection 
	//
	hr = Self()->get_targetSection(&SymULONG);
	if (HR_OK(hr)) {
		Tmp.Format(L"%08X", SymULONG);
		TableRet = InfoTable.Add(SYMBOL_TARGETSECTION, Tmp);
		VERIFY_EXP(TableRet);
	}


	//
	// targetVirtualAddress 
	//
	hr = Self()->get_targetVirtualAddress(&SymULONGLONG);
	if (HR_OK(hr)) {
		Tmp.Format(L"%0.16I64X", SymULONGLONG);
		TableRet = InfoTable.Add(SYMBOL_TARGETVIRTUALADDRESS, Tmp);
		VERIFY_EXP(TableRet);
	}


	//
	// thisAdjust 
	//
	hr = Self()->get_thisAdjust(&SymLONG);
	if (HR_OK(hr)) {
		Tmp.Format(L"%08X", SymLONG);
		TableRet = InfoTable.Add(SYMBOL_THISADJUST, Tmp);
		VERIFY_EXP(TableRet);
	}


	//
	// thunkOrdinal 
	//
	hr = Self()->get_thunkOrdinal(&SymULONG);
	if (HR_OK(hr)) {
		Tmp.Format(L"%08X", SymULONG);
		TableRet = InfoTable.Add(SYMBOL_THISADJUST, Tmp);
		VERIFY_EXP(TableRet);
	}


	//
	// timeStamp 
	//
	hr = Self()->get_timeStamp(&SymULONG);
	if (HR_OK(hr)) {
		Tmp.Format(L"%08X", SymULONG);
		TableRet = InfoTable.Add(SYMBOL_TIMESTAMP, Tmp);
		VERIFY_EXP(TableRet);
	}


	//
	// token 
	//
	hr = Self()->get_token(&SymULONG);
	if (HR_OK(hr)) {
		Tmp.Format(L"%08X", SymULONG);
		TableRet = InfoTable.Add(SYMBOL_TOKEN, Tmp);
		VERIFY_EXP(TableRet);
	}


	//
	// type 
	//
	hr = Self()->get_type(&SymSymbol);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_TYPE, BOOL_STR(SymSymbol != NULL));
		VERIFY_EXP(TableRet);
		SymSymbol->Release();
	}


	//
	// typeId 
	//
	hr = Self()->get_typeId(&SymULONG);
	if (HR_OK(hr)) {
		Tmp.Format(L"%08X", SymULONG);
		TableRet = InfoTable.Add(SYMBOL_TYPEID, Tmp);
		VERIFY_EXP(TableRet);
	}


	//
	// types (Not implement)
	//
	

	//
	// typeIds (Not implement)
	//


	//
	// udtKind 
	//
	hr = Self()->get_udtKind(&SymULONG);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_UDTKIND, GetUdtKindStr(SymULONG));
		VERIFY_EXP(TableRet);
	}


	//
	// unalignedType 
	// 
	hr = Self()->get_unalignedType(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_UNALIGNEDTYPE, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// undecoratedName 
	//
	hr = Self()->get_undecoratedName(&SymBStr);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_UNDECORATEDNAME, SymBStr);
		VERIFY_EXP(TableRet);
	}
	
	
	//
	// undecoratedNameEx (Not implement)
	//


	//
	// upperBound (Not implement)
	//
	

	//
	// upperBoundId (Not implement)
	//


	//
	// value 
	//
	VARIANT SymVar;
	SymVar.vt = VT_EMPTY;
	hr = Self()->get_value(&SymVar);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_VALUE, L"TRUE");
		VERIFY_EXP(TableRet);
	}


	//
	// virtual 
	//
	hr = Self()->get_virtual(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_VIRTUAL, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// virtualAddress 
	//
	hr = Self()->get_virtualAddress(&SymULONGLONG);
	if (HR_OK(hr)) {
		Tmp.Format(L"%0.16I64X", SymULONGLONG);
		TableRet = InfoTable.Add(SYMBOL_VIRTUALADDRESS, Tmp);
		VERIFY_EXP(TableRet);
	}


	//
	// virtualBaseClass 
	//
	hr = Self()->get_virtualBaseClass(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_VIRTUALBASECLASS, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}


	//
	// virtualBaseDispIndex 
	//
	hr = Self()->get_virtualBaseDispIndex(&SymULONG);
	if (HR_OK(hr)) {
		Tmp.Format(L"%08X", SymULONG);
		TableRet = InfoTable.Add(SYMBOL_VIRTUALBASEDISPINDEX, Tmp);
		VERIFY_EXP(TableRet);
	}


	//
	// virtualBaseOffset 
	//
	hr = Self()->get_virtualBaseOffset(&SymULONG);
	if (HR_OK(hr)) {
		Tmp.Format(L"%08X", SymULONG);
		TableRet = InfoTable.Add(SYMBOL_VIRTUALBASEOFFSET, Tmp);
		VERIFY_EXP(TableRet);
	}


	//
	// virtualBasePointerOffset 
	//
	hr = Self()->get_virtualBasePointerOffset(&SymLONG);
	if (HR_OK(hr)) {
		Tmp.Format(L"%08X", SymLONG);
		TableRet = InfoTable.Add(SYMBOL_VIRTUALBASEPOINTEROFFSET, Tmp);
		VERIFY_EXP(TableRet);
	}


	//
	// virtualBaseTableType
	//
	hr = Self()->get_virtualBaseTableType(&SymSymbol);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_VIRTUALBASETABLETYPE, BOOL_STR(SymSymbol != NULL));
		VERIFY_EXP(TableRet);
		SymSymbol->Release();
	}


	//
	// virtualTableShape 
	//
	hr = Self()->get_virtualTableShape(&SymSymbol);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_VIRTUALTABLESHAPE, BOOL_STR(SymSymbol != NULL));
		VERIFY_EXP(TableRet);
		SymSymbol->Release();
	}


	//
	// virtualTableShapeId 
	//
	hr = Self()->get_virtualTableShapeId(&SymULONG);
	if (HR_OK(hr)) {
		Tmp.Format(L"%08X", SymULONG);
		TableRet = InfoTable.Add(SYMBOL_VIRTUALTABLESHAPEID, Tmp);
		VERIFY_EXP(TableRet);
	}


	//
	// volatileType 
	//
	hr = Self()->get_volatileType(&SymBOOL);
	if (HR_OK(hr)) {
		TableRet = InfoTable.Add(SYMBOL_VOLATILETYPE, BOOL_STR(SymBOOL));
		VERIFY_EXP(TableRet);
	}

#ifdef DEBUG_PRINT
	DisplayInfo(InfoTable);
#endif
	
	return S_OK;
}

LPCWSTR SymBase::GetLanguageStr( ULONG Index )
{
	LPCWSTR Lan[] = {
		L"CV_CFL_C",
		L"CV_CFL_CXX",
		L"CV_CFL_FORTRAN",
		L"CV_CFL_MASM",
		L"CV_CFL_PASCAL",
		L"CV_CFL_BASIC",
		L"CV_CFL_COBOL",
		L"CV_CFL_LINK",
		L"CV_CFL_CVTRES",
		L"CV_CFL_CVTPGD",
		L"CV_CFL_CSHARP",
		L"CV_CFL_VB",
		L"CV_CFL_ILASM",
		L"CV_CFL_JAVA",
		L"CV_CFL_JSCRIPT",
		L"CV_CFL_MSIL",
	};

	return Lan[Index];
}

LPCWSTR SymBase::GetLocationTypeStr( ULONG Index )
{
	LPCWSTR LocationTypeStr[] = {
		L"LocIsNull",
		L"LocIsStatic",
		L"LocIsTLS",
		L"LocIsRegRel",
		L"LocIsThisRel",
		L"LocIsEnregistered",
		L"LocIsBitField",
		L"LocIsSlot",
		L"LocIsIlRel",
		L"LocInMetaData",
		L"LocIsConstant",
	};

	return LocationTypeStr[Index];
}

LPCWSTR SymBase::GetMachineTypeStr( ULONG Index )
{
	LPCWSTR MachineTypeStr = NULL;

	switch (Index)
	{
	case CV_CFL_8080: MachineTypeStr = L"CV_CFL_8080"; break;
	case CV_CFL_8086: MachineTypeStr = L"CV_CFL_8086"; break;
	case CV_CFL_80286: MachineTypeStr = L"CV_CFL_80286"; break;
	case CV_CFL_80386: MachineTypeStr = L"CV_CFL_80386"; break;
	case CV_CFL_80486: MachineTypeStr = L"CV_CFL_80486"; break;
	case CV_CFL_PENTIUM: MachineTypeStr = L"CV_CFL_PENTIUM"; break;
	case CV_CFL_PENTIUMII: MachineTypeStr = L"CV_CFL_PENTIUMII"; break;
	case CV_CFL_PENTIUMIII: MachineTypeStr = L"CV_CFL_PENTIUMIII"; break;
	case CV_CFL_MIPS: MachineTypeStr = L"CV_CFL_MIPS"; break;
	case CV_CFL_MIPS16: MachineTypeStr = L"CV_CFL_MIPS16"; break;
	case CV_CFL_MIPS32: MachineTypeStr = L"CV_CFL_MIPS32"; break;
	case CV_CFL_MIPS64: MachineTypeStr = L"CV_CFL_MIPS64"; break;
	case CV_CFL_MIPSI: MachineTypeStr = L"CV_CFL_MIPSI"; break;
	case CV_CFL_MIPSII: MachineTypeStr = L"CV_CFL_MIPSII"; break;
	case CV_CFL_MIPSIII: MachineTypeStr = L"CV_CFL_MIPSIII"; break;
	case CV_CFL_MIPSIV: MachineTypeStr = L"CV_CFL_MIPSIV"; break;
	case CV_CFL_MIPSV: MachineTypeStr = L"CV_CFL_MIPSV"; break;
	case CV_CFL_M68000: MachineTypeStr = L"CV_CFL_M68000"; break;
	case CV_CFL_M68010: MachineTypeStr = L"CV_CFL_M68010"; break;
	case CV_CFL_M68020: MachineTypeStr = L"CV_CFL_M68020"; break;
	case CV_CFL_M68030: MachineTypeStr = L"CV_CFL_M68030"; break;
	case CV_CFL_M68040: MachineTypeStr = L"CV_CFL_M68040"; break;
	case CV_CFL_ALPHA: MachineTypeStr = L"CV_CFL_ALPHA"; break;
	case CV_CFL_ALPHA_21164: MachineTypeStr = L"CV_CFL_ALPHA_21164"; break;
	case CV_CFL_ALPHA_21164A: MachineTypeStr = L"CV_CFL_ALPHA_21164A"; break;
	case CV_CFL_ALPHA_21264: MachineTypeStr = L"CV_CFL_ALPHA_21264"; break;
	case CV_CFL_ALPHA_21364: MachineTypeStr = L"CV_CFL_ALPHA_21364"; break;
	case CV_CFL_PPC601: MachineTypeStr = L"CV_CFL_PPC601"; break;
	case CV_CFL_PPC603: MachineTypeStr = L"CV_CFL_PPC603"; break;
	case CV_CFL_PPC604: MachineTypeStr = L"CV_CFL_PPC604"; break;
	case CV_CFL_PPC620: MachineTypeStr = L"CV_CFL_PPC620"; break;
	case CV_CFL_PPCFP: MachineTypeStr = L"CV_CFL_PPCFP"; break;
	case CV_CFL_SH3: MachineTypeStr = L"CV_CFL_SH3"; break;
	case CV_CFL_SH3E: MachineTypeStr = L"CV_CFL_SH3E"; break;
	case CV_CFL_SH3DSP: MachineTypeStr = L"CV_CFL_SH3DSP"; break;
	case CV_CFL_SH4: MachineTypeStr = L"CV_CFL_SH4"; break;
	case CV_CFL_SHMEDIA: MachineTypeStr = L"CV_CFL_SHMEDIA"; break;
	case CV_CFL_ARM3: MachineTypeStr = L"CV_CFL_ARM3"; break;
	case CV_CFL_ARM4: MachineTypeStr = L"CV_CFL_ARM4"; break;
	case CV_CFL_ARM4T: MachineTypeStr = L"CV_CFL_ARM4T"; break;
	case CV_CFL_ARM5: MachineTypeStr = L"CV_CFL_ARM5"; break;
	case CV_CFL_ARM5T: MachineTypeStr = L"CV_CFL_ARM5T"; break;
	case CV_CFL_ARM6: MachineTypeStr = L"CV_CFL_ARM6"; break;
	case CV_CFL_ARM_XMAC: MachineTypeStr = L"CV_CFL_ARM_XMAC"; break;
	case CV_CFL_ARM_WMMX: MachineTypeStr = L"CV_CFL_ARM_WMMX"; break;
	case CV_CFL_OMNI: MachineTypeStr = L"CV_CFL_OMNI"; break;
	case CV_CFL_IA64: MachineTypeStr = L"CV_CFL_IA64"; break;
	case CV_CFL_IA64_2: MachineTypeStr = L"CV_CFL_IA64_2"; break;
	case CV_CFL_CEE: MachineTypeStr = L"CV_CFL_CEE"; break;
	case CV_CFL_AM33: MachineTypeStr = L"CV_CFL_AM33"; break;
	case CV_CFL_M32R: MachineTypeStr = L"CV_CFL_M32R"; break;
	case CV_CFL_TRICORE: MachineTypeStr = L"CV_CFL_TRICORE"; break;
	case CV_CFL_X64: MachineTypeStr = L"CV_CFL_X64"; break;
	case CV_CFL_EBC: MachineTypeStr = L"CV_CFL_EBC"; break;
	case CV_CFL_THUMB: MachineTypeStr = L"CV_CFL_THUMB"; break;

	default:
		break;
	}

	return MachineTypeStr;
}

LPCWSTR SymBase::GetSymTagStr( ULONG Index )
{
	LPCWSTR SymTagStr[] = {
		L"SymTagNull",
		L"SymTagExe",
		L"SymTagCompiland",
		L"SymTagCompilandDetails",
		L"SymTagCompilandEnv",
		L"SymTagFunction",
		L"SymTagBlock",
		L"SymTagData",
		L"SymTagAnnotation",
		L"SymTagLabel",
		L"SymTagPublicSymbol",
		L"SymTagUDT",
		L"SymTagEnum",
		L"SymTagFunctionType",
		L"SymTagPointerType",
		L"SymTagArrayType",
		L"SymTagBaseType",
		L"SymTagTypedef",
		L"SymTagBaseClass",
		L"SymTagFriend",
		L"SymTagFunctionArgType",
		L"SymTagFuncDebugStart",
		L"SymTagFuncDebugEnd",
		L"SymTagUsingNamespace",
		L"SymTagVTableShape",
		L"SymTagVTable",
		L"SymTagCustom",
		L"SymTagThunk",
		L"SymTagCustomType",
		L"SymTagManagedType",
		L"SymTagDimension"
	};

	return SymTagStr[Index];
}

LPCWSTR SymBase::GetUdtKindStr( ULONG Index )
{
	LPCWSTR UdtKindStr[] = {
		L"UdtStruct",
		L"UdtClass",
		L"UdtUnion"
	};

	return UdtKindStr[Index];
}

void SymBase::DisplayInfo( SymbolInfoTable &InfoTable )
{
	LONG i;
	wprintf(L"----------------------------------------\n");
	for (i = 0; i < InfoTable.GetSize(); i++) {
		wprintf(L"%-40s   %s\n", 
			GetSymbolTypeStr(InfoTable.GetKeyAt(i)), 
			InfoTable.GetValueAt(i).GetString());
	}
}

LPCWSTR SymBase::GetSymbolTypeStr( ULONG Index )
{
	LPCWSTR SymbolTypeStr[] = {
		L"SYMBOL_CHILDREN",
		L"SYMBOL_ACCESS",
		L"SYMBOL_ADDRESSOFFSET",
		L"SYMBOL_ADDRESSSECTION",
		L"SYMBOL_ADDRESSTAKEN",
		L"SYMBOL_AGE",
		L"SYMBOL_ARRAYINDEXTYPE",
		L"SYMBOL_ARRAYINDEXTYPEID",
		L"SYMBOL_BACKENDMAJOR",
		L"SYMBOL_BACKENDMINOR",
		L"SYMBOL_BACKENDBUILD",
		L"SYMBOL_BASETYPE",
		L"SYMBOL_BITPOSITION",
		L"SYMBOL_CALLINGCONVENTION",
		L"SYMBOL_CLASSPARENT",
		L"SYMBOL_CLASSPARENTID",
		L"SYMBOL_CODE",
		L"SYMBOL_COMPILERGENERATED",
		L"SYMBOL_COMPILERNAME",
		L"SYMBOL_CONSTRUCTOR",
		L"SYMBOL_CONTAINER",
		L"SYMBOL_CONSTTYPE",
		L"SYMBOL_COUNT",
		L"SYMBOL_CUSTOMCALLINGCONVENTION",
		L"SYMBOL_DATABYTES",
		L"SYMBOL_DATAKIND",
		L"SYMBOL_EDITANDCONTINUEENABLED",
		L"SYMBOL_FARRETURN",
		L"SYMBOL_FRONTENDMAJOR",
		L"SYMBOL_FRONTENDMINOR",
		L"SYMBOL_FRONTENDBUILD",
		L"SYMBOL_FUNCTION",
		L"SYMBOL_GUID",
		L"SYMBOL_HASALLOCA",
		L"SYMBOL_HASASSIGNMENTOPERATOR",
		L"SYMBOL_HASCASTOPERATOR",
		L"SYMBOL_HASDEBUGINFO",
		L"SYMBOL_HASEH",
		L"SYMBOL_HASEHA",
		L"SYMBOL_HASINLASM",
		L"SYMBOL_HASLONGJUMP",
		L"SYMBOL_HASMANAGEDCODE",
		L"SYMBOL_HASNESTEDTYPES",
		L"SYMBOL_HASSECURITYCHECKS",
		L"SYMBOL_HASSEH",
		L"SYMBOL_HASSETJUMP",
		L"SYMBOL_INDIRECTVIRTUALBASECLASS",
		L"SYMBOL_INLSPEC",
		L"SYMBOL_INTERRUPTRETURN",
		L"SYMBOL_INTRO",
		L"SYMBOL_ISAGGREGATED",
		L"SYMBOL_ISCTYPES",
		L"SYMBOL_ISCVTCIL",
		L"SYMBOL_ISDATAALIGNED",
		L"SYMBOL_ISHOTPATCHABLE",
		L"SYMBOL_ISLTCG",
		L"SYMBOL_ISMSILNETMODULE",
		L"SYMBOL_ISNAKED",
		L"SYMBOL_ISSPLITTED",
		L"SYMBOL_ISSTATIC",
		L"SYMBOL_ISSTRIPPED",
		L"SYMBOL_LANGUAGE",
		L"SYMBOL_LENGTH",
		L"SYMBOL_LEXICALPARENT",
		L"SYMBOL_LEXICALPARENTID",
		L"SYMBOL_LIBRARYNAME",
		L"SYMBOL_LIVELVARINSTANCES",
		L"SYMBOL_LOCATIONTYPE",
		L"SYMBOL_LOWERBOUND",
		L"SYMBOL_LOWERBOUNDID",
		L"SYMBOL_MACHINETYPE",
		L"SYMBOL_MANAGED",
		L"SYMBOL_MSIL",
		L"SYMBOL_NAME",
		L"SYMBOL_NESTED",
		L"SYMBOL_NOINLINE",
		L"SYMBOL_NORETURN",
		L"SYMBOL_NOSTACKORDERING",
		L"SYMBOL_NOTREACHED",
		L"SYMBOL_OBJECTPOINTERTYPE",
		L"SYMBOL_OEMID",
		L"SYMBOL_OEMSYMBOLID",
		L"SYMBOL_OFFSET",
		L"SYMBOL_OPTIMIZEDCODEDEBUGINFO",
		L"SYMBOL_OVERLOADEDOPERATOR",
		L"SYMBOL_PACKED",
		L"SYMBOL_PLATFORM",
		L"SYMBOL_PURE",
		L"SYMBOL_RANK",
		L"SYMBOL_REFERENCE",
		L"SYMBOL_REGISTERID",
		L"SYMBOL_RELATIVEVIRTUALADDRESS",
		L"SYMBOL_SCOPED",
		L"SYMBOL_SIGNATURE",
		L"SYMBOL_SLOT",
		L"SYMBOL_SOURCEFILENAME",
		L"SYMBOL_SYMBOLSFILENAME",
		L"SYMBOL_SYMINDEXID",
		L"SYMBOL_SYMTAG",
		L"SYMBOL_TARGETOFFSET",
		L"SYMBOL_TARGETRELATIVEVIRTUALADDRESS",
		L"SYMBOL_TARGETSECTION",
		L"SYMBOL_TARGETVIRTUALADDRESS",
		L"SYMBOL_THISADJUST",
		L"SYMBOL_THUNKORDINAL",
		L"SYMBOL_TIMESTAMP",
		L"SYMBOL_TOKEN",
		L"SYMBOL_TYPE",
		L"SYMBOL_TYPEID",
		L"SYMBOL_TYPES",
		L"SYMBOL_TYPEIDS",
		L"SYMBOL_UDTKIND",
		L"SYMBOL_UNALIGNEDTYPE",
		L"SYMBOL_UNDECORATEDNAME",
		L"SYMBOL_UNDECORATEDNAMEEX",
		L"SYMBOL_UPPERBOUND",
		L"SYMBOL_UPPERBOUNDID",
		L"SYMBOL_VALUE",
		L"SYMBOL_VIRTUAL",
		L"SYMBOL_VIRTUALADDRESS",
		L"SYMBOL_VIRTUALBASECLASS",
		L"SYMBOL_VIRTUALBASEDISPINDEX",
		L"SYMBOL_VIRTUALBASEOFFSET",
		L"SYMBOL_VIRTUALBASEPOINTEROFFSET",
		L"SYMBOL_VIRTUALBASETABLETYPE",
		L"SYMBOL_VIRTUALTABLESHAPE",
		L"SYMBOL_VIRTUALTABLESHAPEID",
		L"SYMBOL_VOLATILETYPE",
	};

	return SymbolTypeStr[Index];
}

LPCWSTR SymBase::GetCallingConventionStr( ULONG Index )
{
	LPCWSTR CallStr = NULL;
	switch (Index)
	{
	case CV_CALL_NEAR_C: CallStr = L"__cdecl"; break;
	case CV_CALL_NEAR_FAST: CallStr = L"__fastcall"; break;
	case CV_CALL_NEAR_STD: CallStr = L"__stdcall"; break;
	case CV_CALL_NEAR_SYS: CallStr = L"SYSCALL"; break;
	case CV_CALL_THISCALL: CallStr = L"__thiscall"; break;
	case CV_CALL_CLRCALL: CallStr = L"__clrcall"; break;
	default: break;
	}

	ATLASSERT(CallStr != NULL);
	return CallStr;
}

HRESULT SymBase::GetDecl( CString &Info )
{
#ifdef DEBUG_PRINT
	SymbolInfoTable InfoTableDebug;
	SymBase::GetInfoTable(InfoTableDebug);
#endif
	return S_OK;
}

HRESULT SymBase::GetDef( CString &Info )
{
#ifdef DEBUG_PRINT
	SymbolInfoTable InfoTableDebug;
	SymBase::GetInfoTable(InfoTableDebug);
#endif
	return S_OK;
}

void SymBase::EnumEveryChild()
{
	CString Info;
	IDiaEnumSymbols *SymChildren = GetEnum(SymTagNull);
	if (SymChildren != NULL) {
		SymEnumTool SymEnumInst(SymChildren);
		int i;
		for (i = 0; i < SymEnumInst.GetCount(); i++) {
			CAutoPtr<SymBase> p(SymBase::SymNew(SymEnumInst.Item(i)));
			p->GetDecl(Info);
		}
	}
}

LPCWSTR SymBase::GetDataKindStr( ULONG Index )
{
	LPCWSTR DataKindStr[] = {
		L"DataIsUnknown",
		L"DataIsLocal",
		L"DataIsStaticLocal",
		L"DataIsParam",
		L"DataIsObjectPtr",
		L"DataIsFileStatic",
		L"DataIsGlobal",
		L"DataIsMember",
		L"DataIsStaticMember",
		L"DataIsConstant"
	};

	return DataKindStr[Index];
}

LONG SymEnumTool::GetCount()
{
	LONG Count = 0;
	HRESULT hr = _DiaEnumSymbols->get_Count(&Count);
	ATLASSERT(HR_OK(hr));
	return Count;
}

IDiaSymbol* SymEnumTool::Item( ULONG Index )
{
	IDiaSymbol* SubSymbol = NULL;
	HRESULT hr = _DiaEnumSymbols->Item(Index, &SubSymbol);
	ATLASSERT(HR_OK(hr));
	return SubSymbol;
}

HRESULT SymEnumTool::Skip( ULONG Celt )
{
	HRESULT hr = _DiaEnumSymbols->Skip(Celt);
	ATLASSERT(HR_OK(hr));
	return hr;
}

HRESULT SymEnumTool::Reset()
{
	HRESULT hr = _DiaEnumSymbols->Reset();
	ATLASSERT(HR_OK(hr));
	return hr;
}

IDiaSymbol* SymEnumTool::Next()
{
	IDiaSymbol* SubSymbol = NULL;
	ULONG Celt;
	HRESULT hr = _DiaEnumSymbols->Next(1, &SubSymbol, &Celt);
	ATLASSERT(HR_OK(hr));
	return SubSymbol;
}


HRESULT SymFunc::GetDecl(CString &Info)
{
#ifdef DEBUG_PRINT
	SymbolInfoTable InfoTableDebug;
	SymBase::GetInfoTable(InfoTableDebug);
#endif

	BSTR Name;
	HRESULT hr = Self()->get_name(&Name);
	ATLASSERT(HR_OK(hr));
	if (HR_NOK(hr)) {
		return hr;
	}

	CComPtr<IDiaSymbol> FuncType;
	hr = Self()->get_type(&FuncType);
	ATLASSERT(HR_OK(hr));
	if (HR_NOK(hr)) {
		return hr;
	}
 
	CString FuncTypeStr;
	CAutoPtr<SymBase> SubSym(SymBase::SymNew(FuncType));
	hr = SubSym->GetDecl(FuncTypeStr);
	ATLASSERT(HR_OK(hr));

	CString ArgsStr = L"(";
	IDiaEnumSymbols *SymChildren = GetEnum(SymTagData);
	if (SymChildren != NULL) {
		SymEnumTool SymEnumInst(SymChildren);
		int i;
		for (i = 0; i < SymEnumInst.GetCount(); i++) {
			CAutoPtr<SymBase> p(SymBase::SymNew(SymEnumInst.Item(i)));
			ULONG DataKind;
			CString ArgStr;
			hr = p->Self()->get_dataKind(&DataKind);
			if (HR_OK(hr) && DataKind == DataIsParam) {
				if (i != 0) {
					ArgsStr += L", ";
				}
				p->GetDecl(ArgStr);
				ArgsStr += ArgStr;
			}
		}
	}
	ArgsStr += L")";

	Info = FuncTypeStr + L" " + Name + L" " + ArgsStr + L";";

	return hr;
}

HRESULT SymFunc::GetDef( CString &Info )
{
	return GetDecl(Info);
}

HRESULT SymFuncType::GetDecl(CString &Info)
{
#ifdef DEBUG_PRINT
	SymbolInfoTable InfoTableDebug;
	SymBase::GetInfoTable(InfoTableDebug);
#endif

	CComPtr<IDiaSymbol> FuncBaseType;

	HRESULT hr = Self()->get_type(&FuncBaseType);
	ATLASSERT(HR_OK(hr));
	if (HR_NOK(hr)) {
		return hr;
	}

	CAutoPtr<SymBase> SubSym(SymBase::SymNew(FuncBaseType));
	hr = SubSym->GetDecl(Info);
	ATLASSERT(HR_OK(hr));
	
	return hr;
}

HRESULT SymFuncType::GetDef( CString &Info )
{
#ifdef DEBUG_PRINT
	SymbolInfoTable InfoTableDebug;
	SymBase::GetInfoTable(InfoTableDebug);
#endif

	CComPtr<IDiaSymbol> FuncBaseType;

	HRESULT hr = Self()->get_type(&FuncBaseType);
	ATLASSERT(HR_OK(hr));
	if (HR_NOK(hr)) {
		return hr;
	}

	CAutoPtr<SymBase> SubSym(SymBase::SymNew(FuncBaseType));
	hr = SubSym->GetDecl(Info);
	ATLASSERT(HR_OK(hr));
	if (HR_NOK(hr)) {
		return hr;
	}

	Info += L" (";

	ULONG CallConv;
	hr = Self()->get_callingConvention(&CallConv);
	ATLASSERT(HR_OK(hr));
	if (HR_NOK(hr)) {
		return hr;
	}
	Info += GetCallingConventionStr(CallConv);
	Info += L" *)";

	IDiaEnumSymbols *SymChildren = GetEnum(SymTagFunctionArgType);
	Info += L"(";
	if (SymChildren != NULL) {
		SymEnumTool SymEnumInst(SymChildren);
		int i;
		for (i = 0; i < SymEnumInst.GetCount(); i++) {
			if (i != 0) {
				Info += L", ";
			}
			CString Tmp;
			CAutoPtr<SymBase> p(SymBase::SymNew(SymEnumInst.Item(i)));
			p->GetDecl(Tmp);
			Info += Tmp;
		}
	}
	Info += L")";

	return S_OK;
}

HRESULT SymBaseType::GetDecl(CString &Info)
{
	return GetDef(Info);
}


LPCWSTR SymBaseType::GetBaseTypeStr( CComPtr<IDiaSymbol> BaseType )
{
	ULONG Index;

	HRESULT hr = BaseType->get_baseType(&Index);
	ATLASSERT(HR_OK(hr));
	if (HR_NOK(hr)) {
		return NULL;
	}

	ULONGLONG Length;
	hr = BaseType->get_length(&Length);
	ATLASSERT(HR_OK(hr));
	if (HR_NOK(hr)) {
		return NULL;
	}

	return SymBase::GetBaseTypeStr(Index, (ULONG)Length);
}

HRESULT SymBaseType::GetDef( CString &Info )
{
#ifdef DEBUG_PRINT
	SymbolInfoTable InfoTableDebug;
	SymBase::GetInfoTable(InfoTableDebug);
#endif
	LPCWSTR BaseTypeStr = GetBaseTypeStr(Self());
	ATLASSERT(BaseTypeStr != NULL);
	if (BaseTypeStr == NULL) {
		return E_FAIL;
	}

	Info = BaseTypeStr;

	return S_OK;
}



HRESULT SymFuncArgType::GetDecl( CString &Info )
{
#ifdef DEBUG_PRINT
	SymbolInfoTable InfoTableDebug;
	SymBase::GetInfoTable(InfoTableDebug);
#endif

	IDiaSymbol *SymType;
	HRESULT hr = Self()->get_type(&SymType);
	ATLASSERT(HR_OK(hr));
	if (HR_NOK(hr)) {
		return hr;
	}

	CAutoPtr<SymBase> SymSub(SymBase::SymNew(SymType));
	hr = SymSub->GetDecl(Info);

	return hr;
}

HRESULT SymFuncArgType::GetDef( CString &Info )
{
	ATLASSERT(0);
	return E_FAIL;
}

HRESULT SymPointerType::GetDecl( CString &Info )
{
#ifdef DEBUG_PRINT
	SymbolInfoTable InfoTableDebug;
	SymBase::GetInfoTable(InfoTableDebug);
#endif

	IDiaSymbol *SymType;
	HRESULT hr = Self()->get_type(&SymType);
	ATLASSERT(HR_OK(hr));
	if (HR_NOK(hr)) {
		return hr;
	}

	CAutoPtr<SymBase> SymSub(SymBase::SymNew(SymType));
	hr = SymSub->GetDecl(Info);
	ATLASSERT(HR_OK(hr));
	if (HR_OK(hr)) {
		Info += L" *";
	}


	return hr;
}

HRESULT SymPointerType::GetDef( CString &Info )
{
	ATLASSERT(0);
	return E_FAIL;
}

HRESULT SymUDT::GetDecl( CString &Info )
{
#ifdef DEBUG_PRINT
	SymbolInfoTable InfoTableDebug;
	SymBase::GetInfoTable(InfoTableDebug);
#endif

	BSTR DeclName;
	HRESULT hr = Self()->get_name(&DeclName);
	ATLASSERT(HR_OK(hr));
	if (HR_NOK(hr)) {
		return hr;
	}

	CString Name(DeclName);
	if (Name == L"<unnamed-tag>") {
		hr = GetDef(Name);
		ATLASSERT(HR_OK(hr));
	}

	ULONG UdtKind;
	CString UdtName;
	hr = Self()->get_udtKind(&UdtKind);
	ATLASSERT(HR_OK(hr));
	if (HR_NOK(hr)) {
		return hr;
	}


	if (UdtKind == UdtStruct) {
		UdtName = L"struct";
	}
	else if (UdtKind == UdtUnion) {
		UdtName = L"union";
	}
	else {
		UdtName = L"class";
	}

	Info = UdtName + L" " +Name;
	return S_OK;
}

HRESULT SymUDT::GetDef( CString &Info )
{
#ifdef DEBUG_PRINT
	SymbolInfoTable InfoTableDebug;
	SymBase::GetInfoTable(InfoTableDebug);
#endif


	ULONG UdtKind;
	CString Name;
	CString PublicStr;
	CString ProtectStr;
	CString PrivateStr;

	BSTR DeclName;
	HRESULT hr = Self()->get_name(&DeclName);
	ATLASSERT(HR_OK(hr));
	if (HR_NOK(hr)) {
		return hr;
	}

	if (CString(DeclName) != L"<unnamed-tag>") {
		hr = GetDecl(Name);
		ATLASSERT(HR_OK(hr));
		if (HR_NOK(hr)) {
			return hr;
		}
		
	}
	
	hr = Self()->get_udtKind(&UdtKind);
	ATLASSERT(HR_OK(hr));
	if (HR_NOK(hr)) {
		return hr;
	}


	IDiaEnumSymbols *SymChildren = GetEnum(SymTagNull);
	if (SymChildren != NULL) {
		SymEnumTool SymEnumInst(SymChildren);
		int i;
		for (i = 0; i < SymEnumInst.GetCount(); i++) {

			CString Tmp;
			ULONG Access = CV_public;
			CAutoPtr<SymBase> p(SymBase::SymNew(SymEnumInst.Item(i)));

			LONG Offset;
			CString OffsetStr;
			p->Self()->get_offset(&Offset);
			OffsetStr.Format(L" // +0x%X", Offset);

			p->GetDecl(Tmp);
			p->Self()->get_access(&Access);
			if (Access == CV_public) {
				PublicStr += L"\t";
				PublicStr += Tmp + L";";
				PublicStr += OffsetStr;
				PublicStr += L"\r\n";
			}
			else if (Access == CV_protected) {
				ProtectStr += L"\t";
				ProtectStr += Tmp + L";";
				ProtectStr += OffsetStr;
				ProtectStr += L"\r\n";
			}
			else {
				PrivateStr += L"\t";
				PrivateStr += Tmp + L";";
				PrivateStr += OffsetStr;
				PrivateStr += L"\r\n";
			}

		}

	}

	ULONGLONG Length;
	CString LengthStr;
	hr = Self()->get_length(&Length);
	ATLASSERT(HR_OK(hr));
	LengthStr.Format(L" { // Size 0x%I64X\r\n", Length);
	Info = Name + LengthStr;
	if (UdtKind == UdtClass) {
		
		if (PublicStr.GetLength() != 0) {
			Info += L"public:\r\n";
			Info += PublicStr;
		}

		if (ProtectStr.GetLength() != 0) {
			Info += L"protect:\r\n";
			Info += ProtectStr;
		}

		if (PrivateStr.GetLength() != 0) {
			Info += L"private:\r\n";
			Info += PrivateStr;
		}
	}
	else {
		Info += PublicStr;
	}

	Info += L"};";

	return S_OK;
}

HRESULT SymEnum::GetDecl( CString &Info )
{
	BSTR Name;
	HRESULT hr = Self()->get_name(&Name);
	ATLASSERT(HR_OK(hr));
	if (HR_NOK(hr)) {
		return hr;
	}

	Info = L"enum ";
	Info += Name;
	return S_OK;
}

HRESULT SymEnum::GetDef( CString &Info )
{
#ifdef DEBUG_PRINT
	SymbolInfoTable InfoTableDebug;
	SymBase::GetInfoTable(InfoTableDebug);
#endif

	CString Name;
	HRESULT hr = GetDecl(Name);
	ATLASSERT(HR_OK(hr));
	if (HR_NOK(hr)) {
		return hr;
	}


	Info = Name + L" {\r\n";
	IDiaEnumSymbols *SymChildren = GetEnum(SymTagNull);
	if (SymChildren != NULL) {
		SymEnumTool SymEnumInst(SymChildren);
		int i;
		for (i = 0; i < SymEnumInst.GetCount(); i++) {

			BSTR TmpName;
			CComVariant TmpValue;
			CString NameStr;
			CAutoPtr<SymBase> p(SymBase::SymNew(SymEnumInst.Item(i)));
			p->Self()->get_name(&TmpName);
			p->Self()->get_value(&TmpValue);
			Info += L"\t";
			Info += TmpName;
			NameStr.Format(L" = 0x%08X", TmpValue.vt == VT_I4 ? TmpValue.lVal : TmpValue.iVal);
			Info += NameStr;
			Info += L",\r\n";
		}

	}

	Info += L"};";


	return S_OK;
}

HRESULT SymDataType::GetDecl( CString &Info )
{
	return GetDef(Info);
}

HRESULT SymDataType::GetDef( CString &Info )
{
#ifdef DEBUG_PRINT
	SymbolInfoTable InfoTableDebug;
	SymBase::GetInfoTable(InfoTableDebug);
#endif

	IDiaSymbol *SymType;
	HRESULT hr = Self()->get_type(&SymType);
	ATLASSERT(HR_OK(hr));
	if (HR_NOK(hr)) {
		return hr;
	}

	CAutoPtr<SymBase> SymSub(SymBase::SymNew(SymType));
	hr = SymSub->GetDecl(Info);
	ATLASSERT(HR_OK(hr));
	if (HR_NOK(hr)) {
		return hr;
	}

	BSTR Name;
	hr = Self()->get_name(&Name);
	ATLASSERT(HR_OK(hr));
	if (HR_NOK(hr)) {
		return hr;
	}


	int Pos = Info.Find(L'[');
	if (Pos == -1) {
		Info += L" ";
		Info += Name;
	}
	else {
		Info.Insert(Pos, Name);
	}

	ULONG Loctype;
	hr = Self()->get_locationType(&Loctype);
	ATLASSERT(HR_OK(hr));
	if (HR_NOK(hr)) {
		return hr;
	}

	if (Loctype == LocIsBitField) {
		ULONGLONG Length;
		CString LengthStr;
		hr = Self()->get_length(&Length);
		ATLASSERT(HR_OK(hr));
		if (HR_NOK(hr)) {
			return hr;
		}

		LengthStr.Format(L" : %I64d", Length);
		Info += LengthStr;
	}
	

	return hr;
}

HRESULT SymTypedef::GetDef( CString &Info )
{
#ifdef DEBUG_PRINT
	SymbolInfoTable InfoTableDebug;
	SymBase::GetInfoTable(InfoTableDebug);
#endif

	BSTR Name;
	HRESULT hr = Self()->get_name(&Name);
	ATLASSERT(HR_OK(hr));
	if (HR_NOK(hr)) {
		return hr;
	}

	IDiaSymbol *SymType;
	hr = Self()->get_type(&SymType);
	ATLASSERT(HR_OK(hr));
	if (HR_NOK(hr)) {
		return hr;
	}

	CString TypeStr;
	CAutoPtr<SymBase> SymSub(SymBase::SymNew(SymType));
	hr = SymSub->GetDecl(TypeStr);
	ATLASSERT(HR_OK(hr));

	Info = L"typedef ";
	Info += TypeStr + L" " + Name + L";";

	return hr;
}

HRESULT SymTypedef::GetDecl( CString &Info )
{
	return GetDef(Info);
}

HRESULT SymArray::GetDecl( CString &Info )
{
#ifdef DEBUG_PRINT
	SymbolInfoTable InfoTableDebug;
	SymBase::GetInfoTable(InfoTableDebug);
#endif

	IDiaSymbol *SymType;
	HRESULT hr = Self()->get_type(&SymType);
	ATLASSERT(HR_OK(hr));
	if (HR_NOK(hr)) {
		return hr;
	}

	CString TypeStr;
	CAutoPtr<SymBase> SymSub(SymBase::SymNew(SymType));
	hr = SymSub->GetDecl(TypeStr);
	ATLASSERT(HR_OK(hr));

	ULONG Count;
	CString ArrayCount;
	hr = Self()->get_count(&Count);
	ATLASSERT(HR_OK(hr));
	if (HR_NOK(hr)) {
		return hr;
	}

	ArrayCount.Format(L" [%d]", Count);
	Info = TypeStr + ArrayCount;

	return hr;
}

HRESULT SymArray::GetDef( CString &Info )
{
	ATLASSERT(0);
	return E_FAIL;
}
