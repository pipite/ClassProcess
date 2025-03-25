//---------------------------------------------------------------------------


#ifndef ClassProcessH
#define ClassProcessH

#include <System.Classes.hpp>
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <vector>

# pragma comment( lib, "Psapi.lib" )

#include "ClassProcessTool.h"
#include "ClassConvert.h"

class ClassRegion;
class ClassRegions;
class ClassModule;
class ClassModules;
class ClassProcess;
class ClassProcesss;

struct RegionFilter {
	bool WriteCopy;
	bool ExecuteWriteCopy;
	bool ReadOnly;
	bool Execute;
	bool ExecuteRead;
	bool ExecuteReadWrite;
	bool ReadWrite;
	bool Private;
	bool Image;
	bool Mapped;
	bool Guard;
	bool NoCache;
	bool ShowOnlySelectedModule;
	ClassModule *Module;
};

// -----------------------------------------------------
// CLASS ClassRegion
// -----------------------------------------------------
class ClassRegion {
	DWORD         __fastcall GetEnd(void);
	UnicodeString __fastcall GetSExe(void);
	UnicodeString __fastcall GetSCopy(void);
	UnicodeString __fastcall GetSRead(void);
	UnicodeString __fastcall GetSWrite(void);
	UnicodeString __fastcall GetSGuard(void);
	UnicodeString __fastcall GetSNoCache(void);
	UnicodeString __fastcall GetRWOptionStr(void);
	UnicodeString __fastcall GetFilename(void);
	DWORD         __fastcall GetOffset(void);
	bool          __fastcall GetIsStatic(void);

	public:
	DWORD Start;
	DWORD Size;
	UnicodeString Name;
	bool Exe;
	bool Copy;
	bool Read;
	bool Write;
	bool Guard;
	bool NoCache;
	UnicodeString Type;
	unsigned Index;
	ClassModule *Module;

	__fastcall ClassRegion(void);
	__fastcall ~ClassRegion(void);

	__declspec(property(get = GetEnd))DWORD End;
	__declspec(property(get = GetOffset))DWORD Offset;
	__declspec(property(get = GetSExe))UnicodeString SExe;
	__declspec(property(get = GetSCopy))UnicodeString SCopy;
	__declspec(property(get = GetSRead))UnicodeString SRead;
	__declspec(property(get = GetSWrite))UnicodeString SWrite;
	__declspec(property(get = GetSGuard))UnicodeString SGuard;
	__declspec(property(get = GetSNoCache))UnicodeString SNoCache;
	__declspec(property(get = GetRWOptionStr))UnicodeString RWOptionStr;
	__declspec(property(get = GetIsStatic))UnicodeString IsStatic;
	__declspec(property(get = GetFilename))UnicodeString Filename;

};

class ClassRegions {
	ClassProcessTool *Tool;
	ClassConvert     *Cnv;
	ClassProcess     *PProcess;
	ClassModules     *Modules;
	unsigned         Current;

	bool __fastcall SetProcess(ClassProcess *process);
	int  __fastcall GetCount(void);

	public:
	unsigned Size;

	RegionFilter Filter;

	std::vector<ClassRegion*>Region;

	bool __fastcall ClassRegions::EnumStatic(void);
	bool __fastcall ClassRegions::EnumAll(void);
	ClassRegion* __fastcall First(void);
	ClassRegion* __fastcall Next(void);
	ClassRegion* __fastcall Add(void);
	ClassRegion* __fastcall FindByAddress(DWORD address);
	bool         __fastcall Enum(void);

	__fastcall ClassRegions(void);
	__fastcall ~ClassRegions(void);

	void __fastcall Clear(void);

	__declspec(property(get = GetCount))unsigned Count;
	__declspec(property(get = PProcess, put = SetProcess))ClassProcess *Process;
};

// -----------------------------------------------------
// CLASS ClassModule
// -----------------------------------------------------
class ClassModule {
	DWORD __fastcall GetEnd(void);

	public:
	DWORD Start;
	DWORD Size;
	UnicodeString Name;
	UnicodeString Path;
	DWORD Pid;
	DWORD GlblcntUsage;
	DWORD ProccntUsage;
	HANDLE Handle;

	__fastcall ClassModule(void);
	__fastcall ~ClassModule(void);

	__declspec(property(get = GetEnd))DWORD End;
};

class ClassModules {
	ClassProcess  *PProcess;
	unsigned      PSelectedIndex;
	DWORD         ParentPid;
	unsigned      Current;

	bool __fastcall SetProcess(ClassProcess *process);
	void __fastcall SetSelectedIndex(unsigned index);
	bool __fastcall GetSelectedIsValid(void);
	int  __fastcall GetCount(void);

	public:
	std::vector<ClassModule*>Module;
	ClassModule *Selected;

	ClassModule* __fastcall First(void);
	ClassModule* __fastcall Next(void);
	ClassModule* __fastcall Add(void);
	ClassModule* __fastcall FindByAddress(DWORD adress);
	ClassModule* __fastcall FindByName(UnicodeString name);
	void         __fastcall Enum(void);

	__fastcall ClassModules(void);
	__fastcall ~ClassModules(void);

	void __fastcall Clear(void);

	__declspec(property(get = GetCount))unsigned Count;
	__declspec(property(get = PSelectedIndex, put = SetSelectedIndex))unsigned SelectedIndex;
	__declspec(property(get = GetSelectedIsValid))bool SelectedIsValid;
	__declspec(property(get = PProcess, put = SetProcess))ClassProcess *Process;
};

// -----------------------------------------------------
// CLASS ClassProcess
// -----------------------------------------------------
class ClassProcess {
	public:
	DWORD dwSize;
	DWORD th32ProcessID;
	DWORD cntThreads;
	DWORD th32ParentProcessID;
	LONG pcPriClassBase;
	UnicodeString szExeFile;
	HANDLE Handle;

	__fastcall ClassProcess(void);
	__fastcall ~ClassProcess(void);
};

class ClassProcesss {
	unsigned PSelectedIndex;
	unsigned Current;

	int __fastcall GetCount(void);
	void __fastcall SetSelectedIndex(unsigned index);
	bool __fastcall GetSelectedIsValid(void);

	public:
	std::vector<ClassProcess*>Process;
	ClassProcess *Selected;

	ClassProcess* __fastcall First(void);
	ClassProcess* __fastcall Next(void);
	ClassProcess* __fastcall Add(void);
	ClassProcess* __fastcall FindByName(UnicodeString name);
	void          __fastcall Enum(void);

	__fastcall ClassProcesss(void);
	__fastcall ~ClassProcesss(void);

	void __fastcall Clear(void);

	__declspec(property(get = GetCount))unsigned Count;
	__declspec(property(get = PSelectedIndex, put = SetSelectedIndex))unsigned SelectedIndex;
	__declspec(property(get = GetSelectedIsValid))bool SelectedIsValid;
};

#endif
