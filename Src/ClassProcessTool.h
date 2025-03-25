//---------------------------------------------------------------------------
#ifndef ClassProcessToolH
#define ClassProcessToolH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <System.RegularExpressions.hpp>

#include <windows.h>
#include <tlhelp32.h>
#include <WinError.h>

#define STR_MAXSIZE 64

enum STRFORMAT {UCODE, UFT8, ALL};
enum ALIGN {ABYTE=1, AWORD=2, ADWORD=4};
#define IS_LATIN L"^[ a-zA-Z0-9Ó‚Ô‡ÈË‡ÍÁ¬»… !\/\:\'\%\n]*\0$"

// Structure used to communicate data from and to enumeration procedure
struct EnumData {
    DWORD dwProcessId;
	HWND hWnd;
};

class ClassProcessTool {
	TRegEx *RegExpr;

	DWORD PMemoryStart;
	DWORD PMemoryEnd;
	DWORD PPageSize;
	DWORD PGranularity;
	bool  PIA64;

	WORD  *WORDMemBuff;
	DWORD WORDMemStart;
	DWORD WORDMemEnd;
	DWORD WORDMemSize;

	DWORD *DWORDMemBuff;
	DWORD DWORDMemStart;
	DWORD DWORDMemEnd;
	DWORD DWORDMemSize;

	BYTE  *BYTEMemBuff;
	DWORD BYTEMemStart;
	DWORD BYTEMemEnd;
	DWORD BYTEMemSize;

	HMODULE 		__fastcall SearchHiddenModuleHandle(unsigned long pId, char *module);
	UnicodeString 	__fastcall GetMemoryStringMaxLength(HANDLE handle, DWORD address, unsigned lmax, unsigned charsize);
public:
					__fastcall ClassProcessTool(void);
					__fastcall ~ClassProcessTool(void);

	void     	  	__fastcall SetFocus(HWND hwnd);
	HWND     		__fastcall Find_HWND_From_WindowClassName(UnicodeString windowclassname);
	HWND     		__fastcall Find_HWND_From_PID(DWORD pid);
	HWND     		__fastcall Find_HWND_From_HANDLE(HANDLE handle);
	DWORD    		__fastcall Find_PID_From_HWND(HWND hwnd);
	HANDLE   		__fastcall Find_HANDLE_From_ProcessName(UnicodeString exefile);
	DWORD    		__fastcall Find_MemBase_From_PID_ProcessName(DWORD pid, UnicodeString processname);
	wchar_t* 		__fastcall GetSelectedWindowClassName(void);
	DWORD    	  	__fastcall GetModuleSize(HANDLE handle, DWORD lpaddress);

	bool 		  	__fastcall IsReadable(HANDLE handle, DWORD address);

	DWORD         	__fastcall GetDWORD(HANDLE handle, DWORD address);
	WORD          	__fastcall GetWORD(HANDLE handle, DWORD address);
	BYTE          	__fastcall GetBYTE(HANDLE handle, DWORD address);
	float         	__fastcall GetFloat(HANDLE handle, DWORD address);

	DWORD         	__fastcall GetFastDWORD(HANDLE handle, DWORD address);
	WORD          	__fastcall GetFastWORD(HANDLE handle, DWORD address);
	BYTE 		  	__fastcall GetFastBYTE(HANDLE handle, DWORD address);

	UnicodeString 	__fastcall GetString(HANDLE handle, DWORD address, int format);
	UnicodeString 	__fastcall GetStringMaxLength(HANDLE handle, DWORD address, unsigned lmax, int format);

	DWORD 			__fastcall QueryFirstStringAddr(HANDLE handle, UnicodeString searchstring, DWORD addressstart, DWORD addressend, DWORD align, int format);
	DWORD 			__fastcall QueryFirstDwordPtr(HANDLE handle, DWORD searchdword, DWORD addressstart, DWORD addressend);
	DWORD 			__fastcall QueryFirstMaskDwordPtr(HANDLE handle, UnicodeString mask, DWORD addressstart, DWORD addressend);

					__declspec(property(get = PMemoryStart))	DWORD MemoryStart;
					__declspec(property(get = PMemoryEnd))		DWORD MemoryEnd;
					__declspec(property(get = PPageSize))		DWORD PageSize;
					__declspec(property(get = PGranularity))	DWORD Granularity;
					__declspec(property(get = PIA64))			bool  IA64;
};

#endif
