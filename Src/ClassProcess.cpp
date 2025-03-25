//---------------------------------------------------------------------------

#pragma hdrstop

#include "ClassProcess.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

// http://www.tenouk.com/visualcplusmfc/visualcplusmfc20.html

// -----------------------------------------------------
// -----------------------------------------------------
// CLASS ClassRegion
// -----------------------------------------------------
// -----------------------------------------------------
__fastcall ClassRegion::ClassRegion(void) {
	Start = 0;
	Size = 0;
	Name = L"";
	Exe = false;
	Copy = false;
	Read = false;
	Write = false;
	Guard = false;
	NoCache = false;
	Type = L"";
	Index = 0;
	Module = NULL;
}

__fastcall ClassRegion::~ClassRegion(void) {
}

DWORD __fastcall ClassRegion::GetEnd(void) {
	return Start + Size;
}

bool __fastcall ClassRegion::GetIsStatic(void) {
	return Exe;
}

UnicodeString __fastcall ClassRegion::GetSExe(void) {
	if (Exe) return L"E";
	return L"-";
}

UnicodeString __fastcall ClassRegion::GetSCopy(void) {
	if (Copy) return L"C";
	return L"-";
}

UnicodeString __fastcall ClassRegion::GetSRead(void) {
	if (Read) return L"R";
	return L"-";
}

UnicodeString __fastcall ClassRegion::GetSWrite(void) {
	if (Write) return L"W";
	return L"-";
}

UnicodeString __fastcall ClassRegion::GetSGuard(void) {
	if (Guard) return L"G";
	return L"-";
}

UnicodeString __fastcall ClassRegion::GetSNoCache(void) {
	if (NoCache) return L"N";
	return L"-";
}

UnicodeString __fastcall ClassRegion::GetFilename(void) {
	if (Module != NULL) return Module->Name;
	return L"";
}

DWORD __fastcall ClassRegion::GetOffset(void) {
	if (Module != NULL) return Start - Module->Start;
	return 0;
}

UnicodeString __fastcall ClassRegion::GetRWOptionStr(void) {
	return SExe + SCopy + SRead + SWrite + SGuard + SNoCache + L" " + Name;
}

// -----------------------------------------------------
// CLASS ClassRegions
// -----------------------------------------------------
__fastcall ClassRegions::ClassRegions(void) {
	PProcess = NULL;
	Tool = new ClassProcessTool();
	Cnv = new ClassConvert();
	Modules = new ClassModules();

	Current = 0;
	Size = 0;

	Filter.WriteCopy              = true;
	Filter.ExecuteWriteCopy       = true;
	Filter.ReadOnly               = true;
	Filter.Execute                = true;
	Filter.ExecuteRead            = true;
	Filter.ExecuteReadWrite       = true;
	Filter.ReadWrite              = true;
	Filter.Private                = true;
	Filter.Image                  = true;
	Filter.Mapped                 = true;
	Filter.Guard                  = true;
	Filter.NoCache                = true;
	Filter.ShowOnlySelectedModule = false;
	Filter.Module                 = NULL;
}

__fastcall ClassRegions::~ClassRegions(void) {
	Clear();
	delete Tool;
	delete Modules;
	delete Cnv;
}

bool __fastcall ClassRegions::SetProcess(ClassProcess *process) {
	if (process == NULL) return false;
	Modules->Process = process;
	PProcess = process;
	return true;
}

int __fastcall ClassRegions::GetCount(void) {
	return (int) Region.size();
}

void __fastcall ClassRegions::Clear(void) {
	unsigned int i;

	for (i = 0; i < Region.size(); i++) {
		delete Region.at(i);
	}
	Region.erase(Region.begin(), Region.end());
	Current = 0;
}

ClassRegion* __fastcall ClassRegions::First(void) {
	Current = 0;
	if (Region.size() > 0) return Region.at(0);
	return NULL;
}

ClassRegion* __fastcall ClassRegions::Next(void) {
	Current++;
	if (Region.size() > Current) return Region.at(Current);
	return NULL;
}

ClassRegion* __fastcall ClassRegions::Add(void) {
	Region.push_back(new ClassRegion());
	Region.at(Region.size() - 1)->Index = Count;
	return Region.at(Region.size() - 1);
}

ClassRegion* __fastcall ClassRegions::FindByAddress(DWORD address) {
	unsigned int i;

	for (i = 0; i < Region.size(); i++) {
		if ((address >= Region.at(i)->Start) && (address < Region.at(i)->End)) {
			return Region.at(i);
		}
	}
	return NULL;
}

bool __fastcall ClassRegions::EnumStatic(void) {
	if (Process == NULL) return false;
	Filter.WriteCopy              = false;
	Filter.ExecuteWriteCopy       = false;
	Filter.ReadOnly               = false;
	Filter.Execute                = false;
	Filter.ExecuteRead            = false;
	Filter.ExecuteReadWrite       = true;
	Filter.ReadWrite              = false;
	Filter.Private                = false;
	Filter.Image                  = true;
	Filter.Mapped                 = false;
	Filter.Guard                  = false;
	Filter.NoCache                = false;
	Filter.ShowOnlySelectedModule = true;
	Enum();
	if (Count == 0) return false;
	return true;
}

bool __fastcall ClassRegions::EnumAll(void) {
	if (Process == NULL) return false;
	Filter.WriteCopy              = true;
	Filter.ExecuteWriteCopy       = true;
	Filter.ReadOnly               = true;
	Filter.Execute                = true;
	Filter.ExecuteRead            = true;
	Filter.ExecuteReadWrite       = true;
	Filter.ReadWrite              = true;
	Filter.Private                = true;
	Filter.Image                  = true;
	Filter.Mapped                 = true;
	Filter.Guard                  = true;
	Filter.NoCache                = true;
	Filter.ShowOnlySelectedModule = false;
	Enum();
	if (Count == 0) return false;
	return true;
}

bool __fastcall ClassRegions::Enum(void) {
	DWORD start;
	MEMORY_BASIC_INFORMATION mbi;
	ClassRegion *r;
	ClassModule *m;
	bool show;
	wchar_t mf[256];
	int i;
	UnicodeString regionname;

	Size = 0;
	Clear();
	start = Tool->MemoryStart;
	if (Process == NULL) return false;
	while ( (start<Tool->MemoryEnd) && (VirtualQueryEx(Process->Handle,(LPCVOID)start,&mbi,sizeof(mbi)) != 0) && ((start+mbi.RegionSize)>start) ) {
		// Determine if we have access to the page..
		show = true;
		if ( (mbi.State == MEM_COMMIT) && ((mbi.Protect & PAGE_NOACCESS) == 0) /*&& ((mbi.Protect & PAGE_NOCACHE) != PAGE_NOCACHE)*/ ) {
			if (!Filter.WriteCopy)        if ((mbi.Protect & PAGE_WRITECOPY) != 0)          show = false;
			if (!Filter.ExecuteWriteCopy) if ((mbi.Protect & PAGE_EXECUTE_WRITECOPY)  != 0) show = false;
			if (!Filter.Execute)          if ((mbi.Protect & PAGE_EXECUTE) != 0)            show = false;
			if (!Filter.ExecuteRead)      if ((mbi.Protect & PAGE_EXECUTE_READ) != 0)       show = false;
			if (!Filter.ExecuteReadWrite) if ((mbi.Protect & PAGE_EXECUTE_READWRITE) != 0)  show = false;
			if (!Filter.ReadWrite)        if ((mbi.Protect & PAGE_READWRITE) != 0)          show = false;
			if (!Filter.ReadOnly)         if ((mbi.Protect & PAGE_READONLY) != 0)           show = false;

			if (!Filter.Image)            if (mbi.Type == MEM_IMAGE)                        show = false;
			if (!Filter.Mapped)           if (mbi.Type == MEM_MAPPED)                       show = false;
			if (!Filter.Private)          if (mbi.Type == MEM_PRIVATE)                      show = false;

			if (!Filter.Guard)            if ((mbi.Protect & PAGE_GUARD) != 0)              show = false;
			if (!Filter.NoCache)          if (((mbi.Protect & PAGE_NOCACHE) != 0) )         show = false;

			m = Modules->FindByAddress((DWORD)mbi.BaseAddress);
			i = GetMappedFileName(Process->Handle,mbi.BaseAddress, mf, 255);
			if (i != 0) regionname = ExtractFileName(mf); else regionname = Cnv->Dw2Hex((DWORD) mbi.BaseAddress);
			if ( (Filter.Module != NULL) ) {
				if ( Filter.ShowOnlySelectedModule && ( Filter.Module->Name.UpperCase() != regionname.UpperCase() )) show = false;
			}

			if (show) {
				r = Add();
				r->Module = m;
				r->Start = (DWORD) mbi.BaseAddress;
				r->Size = (DWORD) mbi.RegionSize;
				Size += r->Size;

				r->Name = regionname;

				if ((mbi.Protect & PAGE_WRITECOPY) != 0) 		 {r->Copy=true; r->Write=true;}
				if ((mbi.Protect & PAGE_EXECUTE_WRITECOPY) != 0) {r->Exe=true; r->Copy=true; r->Write=true;}
				if ((mbi.Protect & PAGE_EXECUTE) != 0)           {r->Exe=true;}
				if ((mbi.Protect & PAGE_EXECUTE_READ) != 0)      {r->Exe=true; r->Read=true;}
				if ((mbi.Protect & PAGE_EXECUTE_READWRITE) != 0) {r->Exe=true; r->Read=true; r->Write=true;}
				if ((mbi.Protect & PAGE_READWRITE) != 0)         {r->Read=true; r->Write=true;}
				if ((mbi.Protect & PAGE_READONLY) != 0)          {r->Read=true;}
				if ((mbi.Protect & PAGE_GUARD) != 0)             {r->Guard=true;}
				if ((mbi.Protect & PAGE_NOCACHE) != 0)           {r->NoCache=true;}

				if (mbi.Type == MEM_IMAGE) {r->Type = L"IMG";}
				if (mbi.Type == MEM_MAPPED) {r->Type = L"MAP";}
				if (mbi.Type == MEM_PRIVATE) {r->Type = L"PRV";}

			}
		}
		start = start + (DWORD) mbi.RegionSize;
	}
	if (Count > 0) return true;
	return false;
}

// -----------------------------------------------------
// -----------------------------------------------------
// CLASS ClassModule
// -----------------------------------------------------
// -----------------------------------------------------
__fastcall ClassModule::ClassModule(void) {
	Start = 0;
	Size = 0;
	Name = L"";
	Path = L"";
	Pid = 0;
	GlblcntUsage = 0;
	ProccntUsage = 0;
	Handle = 0;
}

__fastcall ClassModule::~ClassModule(void) {
}

DWORD __fastcall ClassModule::GetEnd(void) {
	return Start + Size;
}

// -----------------------------------------------------
// CLASS ClassModules
// -----------------------------------------------------
__fastcall ClassModules::ClassModules(void) {
	PProcess = NULL;
	PSelectedIndex = 0;
	Selected = NULL;
	Current = 0;
}

__fastcall ClassModules::~ClassModules(void) {
	Clear();
}

bool __fastcall ClassModules::SetProcess(ClassProcess *process) {
	if (process == NULL) return false;
	PProcess = process;
	return true;
}

int __fastcall ClassModules::GetCount(void) {
	return (int) Module.size();
}

void __fastcall ClassModules::SetSelectedIndex(unsigned index) {
	if (index < Module.size()) {
		PSelectedIndex = index;
		Selected = Module.at(index);
	} else {
		PSelectedIndex = 0;
		Selected = NULL;
	}
}

bool __fastcall ClassModules::GetSelectedIsValid(void) {
	if (Selected != NULL) return true;
	return false;
}

void __fastcall ClassModules::Clear(void) {
	unsigned int i;

	for (i = 0; i < Module.size(); i++) {
		delete Module.at(i);
	}
	Module.erase(Module.begin(), Module.end());
	Current = 0;
}

ClassModule* __fastcall ClassModules::First(void) {
	Current = 0;
	if (Module.size() > 0) return Module.at(0);
	return NULL;
}

ClassModule* __fastcall ClassModules::Next(void) {
	Current++;
	if (Module.size() > Current) return Module.at(Current);
	return NULL;
}

ClassModule* __fastcall ClassModules::Add(void) {
	Module.push_back(new ClassModule());
	return Module.at(Module.size() - 1);
}

ClassModule* __fastcall ClassModules::FindByAddress(DWORD address) {
	unsigned i;

	for (i = 0; i < Module.size(); i++) {
		if ( (address >= Module.at(i)->Start) && (address <= Module.at(i)->End) ) return Module.at(i);
	}
	return NULL;
}

ClassModule* __fastcall ClassModules::FindByName(UnicodeString name) {
	unsigned i;

	for (i = 0; i < Module.size(); i++) {
		if ( Module.at(i)->Name.UpperCase() == name.UpperCase() ) return Module.at(i);
	}
	return NULL;
}

void __fastcall ClassModules::Enum(void) {
	HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
	MODULEENTRY32 me32;
	ClassModule *m;
	UnicodeString name;

	if (PProcess != NULL) {
		ParentPid = PProcess->th32ProcessID;
		Clear();
		hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, ParentPid);
		if (hModuleSnap == INVALID_HANDLE_VALUE) return;

		me32.dwSize = sizeof(MODULEENTRY32);

		if (!Module32First(hModuleSnap, &me32)) {
			CloseHandle(hModuleSnap);
			return;
		}

		do {
			m = Add();
			m->Name         = UnicodeString(me32.szModule);
			m->Path         = UnicodeString(me32.szExePath);
			m->Pid          = me32.th32ProcessID;
			m->GlblcntUsage = me32.GlblcntUsage;
			m->ProccntUsage = me32.ProccntUsage;
			m->Start        = (DWORD) me32.modBaseAddr;
			m->Size         = me32.modBaseSize;
			m->Handle       = OpenProcess(PROCESS_ALL_ACCESS, 0, m->Pid);

			if (m->Handle != NULL) CloseHandle( m->Handle);
		}
		while (Module32Next(hModuleSnap, &me32));

		CloseHandle(hModuleSnap);
	}
}



// -----------------------------------------------------
// -----------------------------------------------------
// CLASS ClassProcess
// -----------------------------------------------------
// -----------------------------------------------------
__fastcall ClassProcess::ClassProcess(void) {
	dwSize = 0;
	th32ProcessID = 0;
	cntThreads = 0;
	th32ParentProcessID = 0;
	pcPriClassBase = 0L;
	szExeFile = L"";
	Handle = 0;
}

__fastcall ClassProcess::~ClassProcess(void) {

}

// -----------------------------------------------------
// CLASS ClassProcesss
// -----------------------------------------------------
__fastcall ClassProcesss::ClassProcesss(void) {
	PSelectedIndex = 0;
	Selected = NULL;
	Current = 0;
}

__fastcall ClassProcesss::~ClassProcesss(void) {
	Clear();
}

int __fastcall ClassProcesss::GetCount(void) {
	return (int) Process.size();
}

void __fastcall ClassProcesss::SetSelectedIndex(unsigned index) {
	if (index < Process.size()) {
		PSelectedIndex = index;
		Selected = Process.at(index);
	} else {
		PSelectedIndex = 0;
		Selected = NULL;
	}
}

bool __fastcall ClassProcesss::GetSelectedIsValid(void) {
	if (Selected != NULL) return true;
	return false;
}

void __fastcall ClassProcesss::Clear(void) {
	unsigned int i;

	for (i = 0; i < Process.size(); i++) {
		delete Process.at(i);
	}
	Process.erase(Process.begin(), Process.end());
	Current = 0;
}

ClassProcess* __fastcall ClassProcesss::FindByName(UnicodeString name) {
	unsigned int i;

	for (i = 0; i < Process.size(); i++) {
		if ( Process.at(i)->szExeFile.UpperCase() == name.UpperCase() ) return Process.at(i);
	}
	return NULL;
}

ClassProcess* __fastcall ClassProcesss::First(void) {
	Current = 0;
	if (Process.size() > 0) return Process.at(0);
	return NULL;
}

ClassProcess* __fastcall ClassProcesss::Next(void) {
	Current++;
	if (Process.size() > Current) return Process.at(Current);
	return NULL;
}

ClassProcess* __fastcall ClassProcesss::Add(void) {
	Process.push_back(new ClassProcess());
	return Process.at(Process.size() - 1);
}

void __fastcall ClassProcesss::Enum(void) {
	ClassProcess *p;
	HANDLE h;
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	DWORD dwPriorityClass;

	Clear();
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE) return;
	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hProcessSnap, &pe32)) {
		CloseHandle(hProcessSnap);
		return;
	}

	do {
		p = Add();
		dwPriorityClass = 0;
		h = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
		p->Handle = h;
		if (h != NULL) {
			dwPriorityClass = GetPriorityClass(h);
			if (!dwPriorityClass) CloseHandle(h);
		}
		p->dwSize = pe32.dwSize;
		p->th32ProcessID = pe32.th32ProcessID;
		p->cntThreads = pe32.cntThreads;
		p->th32ParentProcessID = pe32.th32ParentProcessID;
		p->pcPriClassBase = pe32.pcPriClassBase;
		p->szExeFile = pe32.szExeFile;
	}
	while (Process32Next(hProcessSnap, &pe32));
	CloseHandle(hProcessSnap);
}


