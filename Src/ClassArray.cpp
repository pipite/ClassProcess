//---------------------------------------------------------------------------

#pragma hdrstop

#include "ClassArray.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

// -----------------------------------------------------
// CLASS ClassArray
// 0 = vide
// SelectedIndex=0 = pas d'elements
// SelectedIndex=1 = dans DWord[0]

// -----------------------------------------------------
__fastcall ClassArray::ClassArray(unsigned size) {
	PMaxSize = size;
	DWord = (DWORD*) malloc(PMaxSize*sizeof(DWORD));
	Clear();
}

__fastcall ClassArray::~ClassArray(void) {
	free(DWord);
}

void __fastcall ClassArray::Clear(void) {
	PCount = 0;
	PCurrent = 0;
}

bool __fastcall ClassArray::GetIsEmpty(void) {
	if (PCount == 0) return true;
	return false;
}

bool __fastcall ClassArray::First(DWORD *retdword) {
	if (PCount > 0) {
		*retdword = DWord[0];
		PCurrent = 1;
		return true;
	}
	PCurrent = 0;
	*retdword = NULL;
	return false;
}

bool __fastcall ClassArray::Next(DWORD *retdword) {
	if ( (PCurrent > 0) && (PCount > PCurrent) ) {
		*retdword = DWord[PCurrent++];
		return true;
	}
	PCurrent = 0;
	*retdword = NULL;
	return false;
}

bool __fastcall ClassArray::Get(unsigned index, DWORD *retdword) {
	if ( (index > 0) && (index <= PCount) ) {
		*retdword = DWord[index-1];
		return true;
	}
	*retdword = NULL;
	return false;
}

bool __fastcall ClassArray::SortAddKey(DWORD dword) {
	int i, j;

	if (PCount >= PMaxSize) return false;

	for (i = 0; i < PCount; i++) {
		if ( DWord[i] == dword) return false;
		if ( DWord[i] < dword ) continue;
		for (j = PCount; j > i; j--) DWord[j] = DWord[j-1];
		break;
	}
	DWord[i] = dword;
	PCount++;
	return true;
}

bool __fastcall ClassArray::SortAdd(DWORD dword) {
	int i, j;

	if (PCount >= PMaxSize) return false;

	for (i = 0; i < PCount; i++) {
		if ( DWord[i] < dword ) continue;
		for (j = PCount; j > i; j--) DWord[j] = DWord[j-1];
		break;
	}
	DWord[i] = dword;
	PCount++;
	return true;
}

bool __fastcall ClassArray::Add(DWORD dword) {
	if (PCount >= PMaxSize) return false;
	DWord[PCount++] = dword;
	return true;
}


bool __fastcall ClassArray::AddKey(DWORD dword) {
	unsigned i;

	if (PCount >= PMaxSize) return false;
	for (i = 0; i < PCount; i++) {
		if ( DWord[i] == dword) return false;
	}
	DWord[PCount++] = dword;
	return true;
}

bool __fastcall ClassArray::GetFirstIndexOf(DWORD dword, unsigned *retindex) {
	unsigned i;

	if (PCount == 0) {
		PCurrent = 0;
		return 0;
	}
	for (i = 0; i < PCount; i++) {
		if ( DWord[i] != dword) continue;
		PCurrent = i+1;
		*retindex = PCurrent;
		return true;
	}
	PCurrent = 0;
	*retindex = PCurrent;
	return false;
}

bool __fastcall ClassArray::GetNextIndexOf(DWORD dword, unsigned *retindex) {
	unsigned i;

	if (PCount == 0) {
		PCurrent = 0;
		return 0;
	}
	for (i = PCurrent; i < PCount; i++) {
		if ( DWord[i] != dword) continue;
		PCurrent = i+1;
		*retindex = PCurrent;
		return true;
	}
	PCurrent = 0;
	*retindex = PCurrent;
	return false;
}

bool __fastcall ClassArray::Delete(unsigned index) {
	unsigned i;

	if ( (index == 0) || (index >= PMaxSize) ) return false;
	if ( PCount == 0 ) return false;
	for (i = index-1; i < PCount; i++) DWord[i] = DWord[i+1];
	PCount--;
	return true;
}


