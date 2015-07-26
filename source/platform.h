#if !defined(PLATFORM_H)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// File:		|	platform.h																			//
// Author:		|	Craig Jeffrey (craigjeffrey3@gmail.com)												//
// Date:		|	25/07/2015																			//
// Description:	|	Platform-independent adaption.														//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#include "type_definitions.h"

//~~~~~~ PLATFORM TO APP SERVICES ~~~~~~\\


//~~~~~~ APP TO PLATFORM SERVICES ~~~~~~\\

struct BitmapBuffer
{
	void* memory;
	int width;
	int height;
	int pitch;
};


void AppUpdate();
void AppRender(BitmapBuffer* buffer);
void ClearBuffer(BitmapBuffer buffer, uint8 red, uint8 green, uint8 blue);

#define PLATFORM_H
#endif