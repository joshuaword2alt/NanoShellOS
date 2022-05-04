// NanoShell Operating System (C) 2022 iProgramInCpp
// Console: E9 debug port
#include <main.h>
#include <console.h>
#include <video.h>
#include <window.h>

void CoE9xClearScreen(UNUSED Console *this)
{
	//send clear screen escape sequence?
}
void CoE9xPlotChar (UNUSED Console *this, UNUSED int x, UNUSED int y, UNUSED char c)
{
	
}
void CoE9xRefreshChar (UNUSED Console *this, UNUSED int x, UNUSED int y)
{
	
}
void CoE9xScrollUpByOne(UNUSED Console *this)
{
	
}
bool CoE9xPrintCharInt (UNUSED Console* this, char c, UNUSED char next, UNUSED bool bDontUpdateCursor)
{
	// Skip all nanoshell specific color codes:
	if (c == 0x01 || c == 0x02)
		return true;
	
	WritePort(0xE9, c);
	return false;
}
void CoE9xUpdateCursor(UNUSED Console* this)
{
	
}
void CoE9xInit(Console *this)
{
	this->curX = this->curY = 0;
	this->width  = 0;
	this->height = 0;
	this->textBuffer = NULL;
	this->pushOrWrap = 0;//push
}

