//---------------------------------------------------------------------------

#ifndef ClassArrayH
#define ClassArrayH

class ClassArray;

#include <System.Classes.hpp>

// -----------------------------------------------------
// CLASS ClassArray (max = 2 147 483 647)
// -----------------------------------------------------
class ClassArray {
	DWORD *DWord;
	unsigned PCount;
	unsigned PCurrent;
	unsigned PMaxSize;

	bool __fastcall GetIsEmpty(void);

public:

	__fastcall ClassArray(unsigned size);
	__fastcall ~ClassArray(void);

	void __fastcall Clear(void);
	bool __fastcall First(DWORD *retdword);
	bool __fastcall Next(DWORD *retdword);

	bool __fastcall Get(unsigned index, DWORD *retdword);
	bool __fastcall GetFirstIndexOf(DWORD dword, unsigned *retindex);
	bool __fastcall GetNextIndexOf(DWORD dword, unsigned *retindex);

	bool __fastcall AddKey(DWORD dword);
	bool __fastcall Add(DWORD dword);
	bool __fastcall SortAdd(DWORD dword);
	bool __fastcall SortAddKey(DWORD dword);

	bool __fastcall Delete(unsigned index);

	__declspec(property(get = PCount))unsigned Count;
	__declspec(property(get = PMaxSize))unsigned MaxSize;
	__declspec(property(get = GetIsEmpty))bool IsEmpty;
};

#endif
