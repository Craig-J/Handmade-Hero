#if !defined(COMMON_H)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// File:		|	common																				//
// Author:		|	Craig Jeffrey (craigjeffrey3@gmail.com)												//
// Date:		|	February 2017																		//
// Description:	|	Commonly required typedefs, definitions, structs, etc.								//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#include <cstdint>
// TODO(Craig): Remove cmath in favor of own math lib.
#include <cmath>

//~~~~~~ PREPROCESSOR ~~~~~~\\

	#define Kilobytes(value) ((value)*1024LL)
	#define Megabytes(value) (Kilobytes(value) * 1024LL)
	#define Gigabytes(value) (Megabytes(value) * 1024LL)
	#define Terabytes(value) (Gigabytes(value) * 1024LL)

	#define ArrayCount(array) (sizeof(array) / sizeof((array)[0]))

	// Use: Declaring static linkage
	#define static_internal static

	// Use: Declaring a local variable as static to persist between calls
	#define static_local_persistant static

	// Use: Declaring an intended global variable
	#define static_global static

	#define Pi32 3.14159265358979323846f

//~~~~~~ BUILD ~~~~~\\

#if !defined(DEV_BUILD)
	#define DEV_BUILD	true
#endif

#if defined(DEBUG_BUILD)
	#define Assert(expression) if(!(expression)) {*(int*)0 = 0;}
#elif defined(RELEASE_BUILD)
	#define Assert(expression)
#endif

//~~~~~~ TYPES ~~~~~~\\

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

//~~~~~~ MISC ~~~~~~\\

	namespace Colour
	{
		struct RGB
		{
			uint8 Red;
			uint8 Green;
			uint8 Blue;
		};
		struct RGBA : public RGB
		{
			uint8 Alpha;
		};

		static_global RGB Black = { 0, 0, 0 };
		static_global RGB White = { 255, 255, 255 };
		static_global RGB Red = { 255, 0, 0 };
		static_global RGB Green = { 0, 255, 0 };
		static_global RGB Blue = { 0, 0, 255 };
	}

	inline uint32 TruncateUInt64(uint64 value)
	{
		Assert(value <= UINT32_MAX);
		uint32 result = (uint32)value;
		return result;
	}

#define COMMON_H
#endif