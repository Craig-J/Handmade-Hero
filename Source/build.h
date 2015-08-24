#if !defined(BUILD_H)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// File:		|	build																				//
// Author:		|	Craig Jeffrey (craigjeffrey3@gmail.com)												//
// Date:		|	August 2015																			//
// Description:	|	Build related definitions and switches.												//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#include "common.h"

#define NO_ASSERT		false
#define INTERNAL_BUILD	true

#if (INTERNAL_BUILD == true) || (DEBUG_BUILD)
	void* DEBUGReadEntireFile(char* filename);
	void DEBUGFreeFileMemory(void* memory);
	bool32 DEBUGWriteEntireFile(char* filename, uint32 memory_size, void* memory);
#else
	
#endif

#if defined(DEBUG_BUILD) && (NO_ASSERT == false)
	#define Assert(expression) if(!(expression)) {*(int*)0 = 0;}
#elif defined(RELEASE_BUILD)
	#define Assert(expression)
#else
	#error "Unknown Build Type"
#endif

#define BUILD_H
#endif