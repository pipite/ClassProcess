//---------------------------------------------------------------------------

#pragma hdrstop

#include "ClassProcessTool.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

// Application-defined callback for EnumWindows
BOOL CALLBACK EnumProc( HWND hWnd, LPARAM lParam ) {
	// Retrieve storage location for communication data
	EnumData& ed = *(EnumData*)lParam;
	DWORD dwProcessId = 0x0;
	// Query process ID for hWnd
	GetWindowThreadProcessId( hWnd, &dwProcessId );
	// Apply filter - if you want to implement additional restrictions,
	// this is the place to do so.
	if ( ed.dwProcessId == dwProcessId ) {
		// Found a window matching the process ID
		ed.hWnd = hWnd;
		// Report success
		System::SetLastError( ERROR_SUCCESS );
		// Stop enumeration
		return FALSE;
	}
	// Continue enumeration
	return TRUE;
}

// ------------------------------------------------
//   ClassProcessTool
// ------------------------------------------------

__fastcall ClassProcessTool::ClassProcessTool(void) {

	SYSTEM_INFO sysInfo = { NULL };

	GetNativeSystemInfo(&sysInfo);

	PPageSize = (DWORD)sysInfo.dwPageSize;
	PMemoryStart = (DWORD)sysInfo.lpMinimumApplicationAddress;
	PMemoryEnd = (DWORD)sysInfo.lpMaximumApplicationAddress;
	PGranularity =(DWORD)sysInfo.dwAllocationGranularity;
	PIA64 = ((sysInfo.wProcessorArchitecture & PROCESSOR_ARCHITECTURE_IA64)||(sysInfo.wProcessorArchitecture & PROCESSOR_ARCHITECTURE_AMD64));

	DWORDMemSize  = PGranularity;
	DWORDMemBuff  = (DWORD*) malloc(DWORDMemSize*sizeof(DWORD));
	DWORDMemStart = NULL;
	DWORDMemEnd   = NULL;

	WORDMemSize   = PGranularity;
	WORDMemBuff   = (WORD*) malloc(WORDMemSize*sizeof(DWORD));
	WORDMemStart  = NULL;
	WORDMemEnd    = NULL;

	BYTEMemSize   = PGranularity;
	BYTEMemBuff   = (BYTE*) malloc(BYTEMemSize*sizeof(DWORD));
	BYTEMemStart  = NULL;
	BYTEMemEnd    = NULL;

	RegExpr = new TRegEx();

}

__fastcall ClassProcessTool::~ClassProcessTool(void) {
	free(WORDMemBuff);
	free(DWORDMemBuff);
	free(BYTEMemBuff);
	delete RegExpr;
}

void __fastcall ClassProcessTool::SetFocus(HWND hwnd) {
	if (hwnd != NULL) {
		SetForegroundWindow(hwnd);
		SetActiveWindow(hwnd);
	}
}

HMODULE __fastcall ClassProcessTool::SearchHiddenModuleHandle(unsigned long pId, char *module) {
	MODULEENTRY32 modEntry;
	HANDLE tlh = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pId);

	modEntry.dwSize = sizeof(MODULEENTRY32);
	Module32First(tlh, &modEntry);

	do {
		if (UnicodeString(modEntry.szModule).UpperCase() == UnicodeString(module).UpperCase() )
			return modEntry.hModule;
		modEntry.dwSize = sizeof(MODULEENTRY32);
	}
	while (Module32Next(tlh, &modEntry));

	return NULL;
}

HWND __fastcall ClassProcessTool::Find_HWND_From_WindowClassName(UnicodeString windowclassname) {
	return FindWindow(windowclassname.c_str(), NULL);
}

HWND __fastcall ClassProcessTool::Find_HWND_From_PID(DWORD pid) {
	EnumData ed = { pid };
	if ( !EnumWindows( EnumProc, (LPARAM)&ed ) &&
         ( GetLastError() == ERROR_SUCCESS ) ) {
		return ed.hWnd;
	}
	return NULL;
}

HWND __fastcall ClassProcessTool::Find_HWND_From_HANDLE( HANDLE handle ) {
	return Find_HWND_From_PID( GetProcessId( handle ) );
}

DWORD __fastcall ClassProcessTool::Find_PID_From_HWND(HWND hwnd) {
	DWORD pid;
	if (hwnd != NULL) {
		GetWindowThreadProcessId(hwnd, &pid);
		return pid;
	}
	return NULL;
}

HANDLE __fastcall ClassProcessTool::Find_HANDLE_From_ProcessName(UnicodeString exefile) {
	HANDLE h;
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	DWORD dwPriorityClass;

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE) return NULL;
	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hProcessSnap, &pe32)) {
		CloseHandle(hProcessSnap);
		return NULL;
	}

	do {
		dwPriorityClass = 0;
		h = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
		if (h != NULL) {
			dwPriorityClass = GetPriorityClass(h);
			if (!dwPriorityClass) CloseHandle(h);
		}
		if (UnicodeString(pe32.szExeFile) == exefile) return h;
	}
	while (Process32Next(hProcessSnap, &pe32));
	CloseHandle(hProcessSnap);
	return NULL;
}

DWORD __fastcall ClassProcessTool::Find_MemBase_From_PID_ProcessName(DWORD pid, UnicodeString processname) {
	if (pid != NULL) {
		return (DWORD)SearchHiddenModuleHandle(pid, AnsiString(processname).c_str());
	}
	return NULL;
}

DWORD __fastcall ClassProcessTool::GetModuleSize(HANDLE handle, DWORD lpaddress)
{
	MEMORY_BASIC_INFORMATION mbi;

	ZeroMemory(&mbi, sizeof(mbi));
	if (VirtualQueryEx(handle, (LPBYTE) lpaddress, &mbi, sizeof(mbi)))
		return mbi.RegionSize;
	else
		return NULL;
}

wchar_t* __fastcall ClassProcessTool::GetSelectedWindowClassName(void) {
	HWND hFore;
	wchar_t* pszBuffer;
	int iNbChar = 1;
	int iNbCharReturned;

	hFore = GetForegroundWindow();

	do {
		iNbChar = iNbChar * 2;
		pszBuffer = new wchar_t[iNbChar];
		iNbCharReturned = GetClassName(hFore, pszBuffer, iNbChar);
		if (iNbCharReturned < (iNbChar - 1)) {
			return pszBuffer;
		}
		delete[]pszBuffer;
	}
	while ((iNbCharReturned >= (iNbChar - 1)));
	return NULL;
}

// ----------------  Read  --------------------

bool __fastcall ClassProcessTool::IsReadable(HANDLE handle, DWORD address) {
	DWORD r;

	if (address < PMemoryStart || address > PMemoryEnd) return false;
	try {
		return ReadProcessMemory(handle, (LPCVOID*)(address), &r, sizeof(DWORD), 0);
	} catch (Exception &E) {
		return false;
	}
}

DWORD __fastcall ClassProcessTool::GetDWORD(HANDLE handle, DWORD address) {
	DWORD r;

	if (address < PMemoryStart) return NULL;
	try {
		ReadProcessMemory(handle, (LPCVOID*)(address), &r, sizeof(DWORD), 0);
		return r;
	} catch (Exception &E) {
		return NULL;
	}
}

WORD __fastcall ClassProcessTool::GetWORD(HANDLE handle, DWORD address) {
	WORD r;

	if (address < PMemoryStart) return NULL;
	try {
		ReadProcessMemory(handle, (LPCVOID*)(address), &r, sizeof(WORD), 0);
		return r;
	} catch (Exception &E) {
		return NULL;
	}
}

float __fastcall ClassProcessTool::GetFloat(HANDLE handle, DWORD address) {
	float r;

	if (address < PMemoryStart) return NULL;
	try {
		ReadProcessMemory(handle, (LPCVOID*)(address), &r, sizeof(float), 0);
		return r;
	} catch (Exception &E) {
		return NULL;
	}
}

BYTE __fastcall ClassProcessTool::GetBYTE(HANDLE handle, DWORD address) {
	BYTE r;

	if (address < PMemoryStart) return NULL;
	try {
		ReadProcessMemory(handle, (LPCVOID*)(address), &r, sizeof(BYTE), 0);
		return r;
	} catch (Exception &E) {
		return NULL;
	}
}

DWORD __fastcall ClassProcessTool::GetFastDWORD(HANDLE handle, DWORD address) {
	if (address < PMemoryStart) return NULL;
	if ( (address < DWORDMemStart) || (address > (DWORDMemEnd) )  ) {
		DWORDMemStart = address;
		DWORDMemEnd = address + DWORDMemSize*sizeof(DWORD);
		try {
			ReadProcessMemory(handle, (LPCVOID*)(address), DWORDMemBuff, sizeof(DWORD)*DWORDMemSize, 0);
		} catch (Exception &E) {
			return NULL;
		}
	}
	return DWORDMemBuff[(DWORD)(address-DWORDMemStart)/ALIGN::ADWORD];
}

WORD __fastcall ClassProcessTool::GetFastWORD(HANDLE handle, DWORD address) {
	if (address < PMemoryStart) return NULL;
	if ( (address < WORDMemStart) || (address > (WORDMemEnd) )  ) {
		WORDMemStart = address;
		WORDMemEnd = address + WORDMemSize*sizeof(WORD);
		ReadProcessMemory(handle, (LPCVOID*)(address), WORDMemBuff, sizeof(WORD)*WORDMemSize, 0);
	}
	return WORDMemBuff[(address-WORDMemStart)/ALIGN::AWORD];
}

BYTE __fastcall ClassProcessTool::GetFastBYTE(HANDLE handle, DWORD address) {
	if (address < PMemoryStart) return NULL;
	if ( (address < BYTEMemStart) || (address > (BYTEMemEnd) )  ) {
		BYTEMemStart = address;
		BYTEMemEnd = address + BYTEMemSize;
		ReadProcessMemory(handle, (LPCVOID*)(address), BYTEMemBuff, BYTEMemSize, 0);
	}
	return BYTEMemBuff[(address-BYTEMemStart)];
}

UnicodeString __fastcall ClassProcessTool::GetString(HANDLE handle, DWORD address, int format) {
	return GetStringMaxLength(handle, address, STR_MAXSIZE, format);
}

UnicodeString __fastcall ClassProcessTool::GetStringMaxLength(HANDLE handle, DWORD address, unsigned lmax, int format) {
	UnicodeString s;
	DWORD sz, sm;

	if (!address) return L"";
	sm = (lmax <= STR_MAXSIZE ? lmax : STR_MAXSIZE);
	switch (format) {
		case STRFORMAT::UCODE : {
			sz = sm+sm;
			ReadProcessMemory(handle, (LPCVOID*)(address), BYTEMemBuff, sz, 0);
			BYTEMemBuff[sz] = '\0';
			return UnicodeString( AnsiString( PWideChar(BYTEMemBuff) ) );

		}
		case STRFORMAT::UFT8  : {
			ReadProcessMemory(handle, (LPCVOID*)(address), BYTEMemBuff, sm, 0);
			BYTEMemBuff[sm] = '\0';
			return UnicodeString( (char*)BYTEMemBuff );
		}
		case STRFORMAT::ALL : {
			sz = sm+sm;
			ReadProcessMemory(handle, (LPCVOID*)(address), BYTEMemBuff, sz, 0);
			// Try UnicodeString
			BYTEMemBuff[sz] = '\0';
			s = UnicodeString( AnsiString( PWideChar(BYTEMemBuff) ) );
			if ( RegExpr->IsMatch(s,IS_LATIN) ) return s;
			// Try UFT8
			BYTEMemBuff[sm] = '\0';
			s = UnicodeString( (char*)BYTEMemBuff );
			if ( RegExpr->IsMatch(s,IS_LATIN) ) return s;
		}
		default: return L"";
	}
}

// ----------------  Find  --------------------

DWORD __fastcall ClassProcessTool::QueryFirstStringAddr(HANDLE handle, UnicodeString searchstring, DWORD addressstart, DWORD addressend, DWORD align, int format) {
	DWORD i, j, l;
	bool found_unicode, found_uft8;
	bool to_unicode = true;
	bool to_uft8 = true;
	char a, c;
	char s[STR_MAXSIZE];

	strcpy( s, AnsiString(searchstring.SubString(1,STR_MAXSIZE)).c_str() );
	l = searchstring.Length();
	switch (format) {
		case STRFORMAT::UCODE : { to_uft8    = false; break; };
		case STRFORMAT::UFT8  : { to_unicode = false; break; };
	}

	for (i = addressstart; i < addressend; i = i + align ) {
		found_unicode = to_unicode;
		found_uft8 = to_uft8;
		for (j = 0; j < l; j++) {
			c = s[j];
			// Try Unicode
			if ( found_unicode ) {
				a = (char) GetFastBYTE(handle, i + j*ALIGN::AWORD);
				if (a != c) found_unicode = false;
			}
			// Try UFT8
			if ( found_uft8 ) {
				a = (char) GetFastBYTE(handle, i + j);
				if (a != c) found_uft8 = false;
			}
			if ( !found_unicode && !found_uft8 ) break;
		}
		if ( j == l ) return i;
	}
	return NULL;
}

DWORD __fastcall ClassProcessTool::QueryFirstDwordPtr(HANDLE handle, DWORD searchdword, DWORD addressstart, DWORD addressend) {
	// Renvoie le premier pointeur pointant sur searchdword
	DWORD i;

	for (i = addressstart; i < addressend; i = i + ALIGN::ADWORD) {
		if (GetFastDWORD(handle, i) == searchdword) return i;
	}

	return NULL;
}

DWORD __fastcall ClassProcessTool::QueryFirstMaskDwordPtr(HANDLE handle, UnicodeString mask, DWORD addressstart, DWORD addressend) {
	// Renvoie le premier pointeur pointant sur searchdword
	DWORD i, j;
	UnicodeString smask, smasksearch;
	DWORD maskvalue, masksearch, test;

	smasksearch = mask;
	smask       = L"FFFFFFFF";
	j = 8;
	for (i = smasksearch.Length(); i > 0  ; i--) {
		if (mask[i] == '?') {
			smask[j]       = '0';
			smasksearch[i] = '0';
		}
		j--;
	}

	try {
//		test = 0x12345678;
		maskvalue  = (DWORD) StrToInt(L"0X"+smask);
		masksearch = (DWORD) StrToInt(L"0X"+smasksearch);
//		test = test & maskvalue;
		for (i = addressstart; i < addressend; i = i + ALIGN::ADWORD) {
			if ( (GetFastDWORD(handle, i) & maskvalue) == masksearch) return i;
		}
	} catch (...) {
		return NULL;
	}

	return NULL;
}


