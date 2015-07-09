//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// File:			type_definitions.h																	//
// Author:			Craig Jeffrey (craigjeffrey3@gmail.com)												//
// Date:			10/07/2015																			//
// Description:		Personalized/extended definitions for C/C++ intrinsic and fundamental types.		//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#ifndef _TYPE_DEFINITIONS_
#define _TYPE_DEFINITIONS_

#include <cstdint>

//~~~~~~ PREPROCESSOR ~~~~~~//

	// Use: Declaring static linkage
	// Rationale: Disambiguation
	#define static_internal static

	// Use: Declaring a local variable as static to persist between calls
	// Rationale: Disambiguation
	#define static_local_persistant static

	// Use: Declaring an intended global variable
	// Rationale: Disambiguation
	#define static_global static

//_______PREPROCESSOR_______//
//~~~~~~ TYPEDEFS ~~~~~~//

	typedef uint8_t ui8;
	typedef uint16_t ui16;
	typedef uint32_t ui32;
	typedef uint64_t ui64;

	typedef int8_t i8;
	typedef int16_t i16;
	typedef int32_t i32;
	typedef int64_t i64;

//_______TYPEDEFS_______//

#endif