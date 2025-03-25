//---------------------------------------------------------------------------

#pragma hdrstop

#include "ClassMem.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

// -----------------------------------------------------
// CLASS ClassMem
// -----------------------------------------------------
__fastcall ClassMem::ClassMem(void) {
	ProcessTool = new ClassProcessTool();
	Processs    = new ClassProcesss();
	Modules     = new ClassModules();
	Regions     = new ClassRegions();
	Cnv         = new ClassConvert();
}

__fastcall ClassMem::~ClassMem(void) {
	delete Cnv;
	delete Regions;
	delete Modules;
	delete Processs;
	delete ProcessTool;
}
