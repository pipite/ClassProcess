//---------------------------------------------------------------------------

#ifndef ClassMemH
#define ClassMemH
//---------------------------------------------------------------------------

class ClassMem;

#include "ClassProcess.h"
#include "ClassProcessTool.h"
#include "ClassConvert.h"

// -----------------------------------------------------
// CLASS THexEdit
// -----------------------------------------------------
class ClassMem {

public:
	ClassProcessTool *ProcessTool;
	ClassConvert     *Cnv;
	ClassRegions     *Regions;
	ClassModules     *Modules;
	ClassProcesss    *Processs;

	__fastcall ClassMem(void);
	__fastcall ~ClassMem(void);
};


#endif
