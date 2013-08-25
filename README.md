SymWrap
=======

A class for dia(Debug Interface Access)



### Sample

		CString PdbName = L"sample.pdb";
		CString Regexp = L"_PEB";
		enum SymTagEnum Type = SymTagUDT; // or SymTagFunction
		
		SymLoader Loader;
		HRESULT hr = Loader.Init();
		if (HR_NOK(hr)) {
			wprintf(L"Initialize Loader failed.\n");
			return;
		}
		hr = Loader.Open((LPWSTR)PdbName.GetString());
		if (HR_NOK(hr)) {
			wprintf(L"Open pdb failed.\n");
			return;
		}
		IDiaSymbol *BaseSymbol = Loader.GetSymbol();
		if (BaseSymbol == NULL) {
			wprintf(L"Get base symbol failed.\n");
			return;
		}
		SymBase Base(BaseSymbol);
		IDiaEnumSymbols* EnumSymbols = Base.GetEnum(Type, (LPWSTR)Regexp.GetString(), nsCaseInRegularExpression);
		if (EnumSymbols == NULL) {
			wprintf(L"Get enum symbol failed.\n");
			return;
		}
		SymEnumTool Enumer(EnumSymbols);
		LONG Count = Enumer.GetCount();
		for (i = 0; i < Count; i++) {
			CAutoPtr<SymBase> p(SymBase::SymNew(Enumer.Item(i)));
			CString Str;
			p->GetDef(Str);
			wprintf(Str);
			wprintf(L"\n");
		}
