#if !defined(TYPE_DEFINITIONS_H)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// File:		|	type_definitions.h																	//
// Author:		|	Craig Jeffrey (craigjeffrey3@gmail.com)												//
// Date:		|	10/07/2015																			//
// Description:	|	Customized definitions for C/C++ intrinsic and fundamental types.					//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#include <cstdint>

//~~~~~~ PREPROCESSOR ~~~~~~\\

	// Use: Declaring static linkage
	#define static_internal static

	// Use: Declaring a local variable as static to persist between calls
	#define static_local_persistant static

	// Use: Declaring an intended global variable
	#define static_global static

	#define Pi32 3.14159265358979323846f

//~~~~~~ TYPEDEFS ~~~~~~\\

	typedef uint8_t uint8;
	typedef uint16_t uint16;
	typedef uint32_t uint32;
	typedef uint64_t uint64;

	typedef int8_t int8;
	typedef int16_t int16;
	typedef int32_t int32;
	typedef int64_t int64;

	typedef int32 bool32;

	typedef float real32;
	typedef double real64;

#define TYPE_DEFINITIONS_H
#endif