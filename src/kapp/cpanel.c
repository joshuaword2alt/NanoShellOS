/*****************************************
		NanoShell Operating System
	      (C) 2022 iProgramInCpp

     Control panel Application module
******************************************/

#include <wbuiltin.h>
#include <wterm.h>
#include <vfs.h>
#include <elf.h>
#include <keyboard.h>

extern VBEData g_mainScreenVBEData, *g_vbeData;
extern bool    g_ps2MouseAvail;
extern void PaintWindowBorderNoBackgroundOverpaint(Window* pWindow);

extern bool     g_BackgroundSolidColorActive, g_RenderWindowContents;
extern uint32_t g_BackgroundSolidColor;

void RedrawEverything();
//POPUP WINDOWS: Set `pWindow->m_data` to anything to exit.
#define MOUSE_POPUP_WINDOW 1//stubbed out for now because it's buggy as hell
#define KEYBD_POPUP_WINDOW 1
#define DESKT_POPUP_WINDOW 1

#if MOUSE_POPUP_WINDOW
	enum
	{
		MOUSEP_SPEED_SCROLL = 1000,
		MOUSEP_KEYBOARD_CONTROL_MOUSE,
	};
	int  GetMouseSpeedMultiplier();
	void SetMouseSpeedMultiplier(int spd);
	#define MOUSE_POPUP_WIDTH 200
	#define MOUSE_POPUP_HEITE 70
	void CALLBACK Cpl$MousePopupWndProc(Window* pWindow, int messageType, int parm1, int parm2)
	{
		switch (messageType)
		{
			case EVENT_CREATE:
			{
				pWindow->m_iconID = ICON_MOUSE;//TODO
				
				//add a button
				Rectangle r;
				RECT(r,8,8+TITLE_BAR_HEIGHT,MOUSE_POPUP_WIDTH-16,35);
				AddControl(pWindow, CONTROL_SURROUND_RECT, r, "Mouse tracking speed", 1, 0, 0);
				{
					//add stuff inside the rect.
					//this scope has no actual reason for its existence other than to mark that stuff we add here goes inside the rect above.
					
					RECT(r, 16,  24 + TITLE_BAR_HEIGHT, 32, 20);
					AddControl(pWindow, CONTROL_TEXT, r, "Slow", 2, 0, WINDOW_BACKGD_COLOR);
					RECT(r, MOUSE_POPUP_WIDTH - 40, 24 + TITLE_BAR_HEIGHT, 32, 20);
					AddControl(pWindow, CONTROL_TEXT, r, "Fast", 3, 0, WINDOW_BACKGD_COLOR);
					RECT(r, 50,  22 + TITLE_BAR_HEIGHT, MOUSE_POPUP_WIDTH - 100, 1);
					AddControl(pWindow, CONTROL_HSCROLLBAR, r, NULL, MOUSEP_SPEED_SCROLL, (0)<<16|(4), (1)<<16|(GetMouseSpeedMultiplier()));
				}
				
				break;
			}
			case EVENT_COMMAND:
				DestroyWindow(pWindow);
				break;
			case EVENT_RELEASECURSOR:
				SetMouseSpeedMultiplier(GetScrollBarPos(pWindow, MOUSEP_SPEED_SCROLL));
				break;
			default:
				DefaultWindowProc(pWindow, messageType, parm1, parm2);
				break;
		}
	}
#endif
#if DESKT_POPUP_WINDOW
	enum
	{
		DESKTOP_ENABLE_BACKGD = 4000,
		DESKTOP_SHOW_WINDOW_CONTENTS,
		DESKTOP_APPLY_CHANGES,
		DESKTOP_CHANGE_BACKGD,
		DESKTOP_CANCEL,
	};
	#define DESKTOP_POPUP_WIDTH 300
	#define DESKTOP_POPUP_HEITE 140
	void CALLBACK Cpl$DesktopPopupWndProc(Window* pWindow, int messageType, int parm1, int parm2)
	{
		switch (messageType)
		{
			case EVENT_CREATE:
			{
				pWindow->m_iconID = ICON_DESKTOP;//TODO
				
				//add a button
				Rectangle r;
				RECT(r,10, 10 + TITLE_BAR_HEIGHT, DESKTOP_POPUP_WIDTH - 150, 15);
				AddControl(pWindow, CONTROL_CHECKBOX, r, "Solid color background", DESKTOP_ENABLE_BACKGD, g_BackgroundSolidColorActive, 0);
				RECT(r,DESKTOP_POPUP_WIDTH-80, 5 + TITLE_BAR_HEIGHT, 70, 20);
				AddControl(pWindow, CONTROL_BUTTON,   r, "Change...", DESKTOP_CHANGE_BACKGD, 0, 0);
				RECT(r,10, 30 + TITLE_BAR_HEIGHT, DESKTOP_POPUP_WIDTH - 20, 15);
				AddControl(pWindow, CONTROL_CHECKBOX, r, "Show window contents while moving", DESKTOP_SHOW_WINDOW_CONTENTS, g_RenderWindowContents, 0);
				
				RECT(r,(DESKTOP_POPUP_WIDTH-100)/2,8+TITLE_BAR_HEIGHT+80,45,20);
				AddControl(pWindow, CONTROL_BUTTON, r, "Cancel", DESKTOP_CANCEL, 0, 0);
				RECT(r,(DESKTOP_POPUP_WIDTH-100)/2+55,8+TITLE_BAR_HEIGHT+80,45,20);
				AddControl(pWindow, CONTROL_BUTTON, r, "OK",  DESKTOP_APPLY_CHANGES, 0, 0);
				
				break;
			}
			case EVENT_COMMAND:
				if (parm1 == DESKTOP_CHANGE_BACKGD)
				{
					uint32_t data = ColorInputBox(pWindow, "Choose a new background color:", "Background color");
					if (data != TRANSPARENT)
					{
						g_BackgroundSolidColor = data & 0xffffff;
						RedrawEverything();
					}
					break;
				}
				if (parm1 == DESKTOP_APPLY_CHANGES)
				{
					g_BackgroundSolidColorActive = CheckboxGetChecked(pWindow, DESKTOP_ENABLE_BACKGD);
					g_RenderWindowContents       = CheckboxGetChecked(pWindow, DESKTOP_SHOW_WINDOW_CONTENTS);
					RedrawEverything();
				}
				DestroyWindow(pWindow);
				break;
			case EVENT_RELEASECURSOR:
				
				break;
			default:
				DefaultWindowProc(pWindow, messageType, parm1, parm2);
				break;
		}
	}
#endif
#if KEYBD_POPUP_WINDOW
	enum
	{
		KEYBDP_REPEAT_CPS = 1000,
		KEYBDP_REPEAT_DELAY,
		KEYBDP_OK_BUTTON,
		KEYBDP_CANCEL_BUTTON,
		KEYBDP_TEST_BOX,
	};
	#define KEYBD_POPUP_WIDTH 300
	#define KEYBD_POPUP_HEITE 140
	
	uint8_t g_oldTypematicRepeatRate, g_oldTypematicRepeatDelay;
	void CALLBACK Cpl$KeybdPopupWndProc(Window* pWindow, int messageType, int parm1, int parm2)
	{
		switch (messageType)
		{
			case EVENT_CREATE:
			{
				pWindow->m_iconID = ICON_KEYBOARD;//TODO
				
				g_oldTypematicRepeatRate  = GetKeyboardProperty(KBPROPERTY_REPEAT_FREQUENCY);
				g_oldTypematicRepeatDelay = GetKeyboardProperty(KBPROPERTY_DELAY_BEFORE_REPEAT);
				
				//add a button
				Rectangle r;
				RECT(r,8,8+TITLE_BAR_HEIGHT,KEYBD_POPUP_WIDTH-16,35);
				AddControl(pWindow, CONTROL_SURROUND_RECT, r, "Keyboard repeat rate", 1, 0, 0);
				{
					//add stuff inside the rect.
					//this scope has no actual reason for its existence other than to mark that stuff we add here goes inside the rect above.
					
					RECT(r, 16,  24 + TITLE_BAR_HEIGHT, 32, 20);
					AddControl(pWindow, CONTROL_TEXT, r, "Slow", 2, 0, WINDOW_BACKGD_COLOR);
					RECT(r, KEYBD_POPUP_WIDTH - 40, 24 + TITLE_BAR_HEIGHT, 32, 20);
					AddControl(pWindow, CONTROL_TEXT, r, "Fast", 3, 0, WINDOW_BACKGD_COLOR);
					RECT(r, 50,  22 + TITLE_BAR_HEIGHT, KEYBD_POPUP_WIDTH - 100, 1);
					AddControl(pWindow, CONTROL_HSCROLLBAR, r, NULL, KEYBDP_REPEAT_CPS,
						(0)<<16|(GetKeyboardProperty(KBPROPERTY_REPEAT_FREQUENCY_MAX)),
						(1)<<16|(GetKeyboardProperty(KBPROPERTY_REPEAT_FREQUENCY_MAX)-1-g_oldTypematicRepeatRate)
					);
				}
				RECT(r,8,8+TITLE_BAR_HEIGHT+40,KEYBD_POPUP_WIDTH-16,35);
				AddControl(pWindow, CONTROL_SURROUND_RECT, r, "Delay before repeat", 1, 0, 0);
				{
					//add stuff inside the rect.
					//this scope has no actual reason for its existence other than to mark that stuff we add here goes inside the rect above.
					
					RECT(r, 16,  24+40 + TITLE_BAR_HEIGHT, 32, 20);
					AddControl(pWindow, CONTROL_TEXT, r, "Slow", 2, 0, WINDOW_BACKGD_COLOR);
					RECT(r, KEYBD_POPUP_WIDTH - 40, 24+40 + TITLE_BAR_HEIGHT, 32, 20);
					AddControl(pWindow, CONTROL_TEXT, r, "Fast", 3, 0, WINDOW_BACKGD_COLOR);
					RECT(r, 50,  22+40 + TITLE_BAR_HEIGHT, KEYBD_POPUP_WIDTH - 100, 1);
					AddControl(pWindow, CONTROL_HSCROLLBAR, r, NULL, KEYBDP_REPEAT_DELAY,
						(0)<<16|(GetKeyboardProperty(KBPROPERTY_DELAY_BEFORE_REPEAT_MAX)),
						(1)<<16|(GetKeyboardProperty(KBPROPERTY_DELAY_BEFORE_REPEAT_MAX)-1-g_oldTypematicRepeatDelay)
					);
				}
				RECT(r,(KEYBD_POPUP_WIDTH-100)/2,8+TITLE_BAR_HEIGHT+80,45,20);
				AddControl(pWindow, CONTROL_BUTTON, r, "Revert", KEYBDP_CANCEL_BUTTON, 0, 0);
				RECT(r,(KEYBD_POPUP_WIDTH-100)/2+55,8+TITLE_BAR_HEIGHT+80,45,20);
				AddControl(pWindow, CONTROL_BUTTON, r, "Apply",  KEYBDP_OK_BUTTON,     0, 0);
				/*
				RECT(r,8,8+TITLE_BAR_HEIGHT+80,KEYBD_POPUP_WIDTH-16,80);
				AddControl(pWindow, CONTROL_TEXTINPUT, r, NULL, KEYBDP_TEST_BOX, 0, 0);
				SetTextInputText(pWindow, KEYBDP_TEST_BOX, "Test");
				*/
				break;
			}
	#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"//fallthrough intentional.
			case EVENT_COMMAND:
				if (parm1 != KEYBDP_CANCEL_BUTTON)
				{
					if (parm1 != KEYBDP_OK_BUTTON)
						break;
					
					//ok button:
					messageType = EVENT_CLOSE;
					//fallthrough
				}
				else
				{
					//cancel button:
					SetKeyboardProperty(KBPROPERTY_DELAY_BEFORE_REPEAT,  g_oldTypematicRepeatDelay);
					SetKeyboardProperty(KBPROPERTY_REPEAT_FREQUENCY,     g_oldTypematicRepeatRate);
					FlushKeyboardProperties();
					messageType = EVENT_CLOSE;
					//fallthrough
				}
				//fallthrough
			case EVENT_CLOSE:
				DefaultWindowProc(pWindow, messageType, parm1, parm2);
				break;
	#pragma GCC diagnostic pop
			case EVENT_RELEASECURSOR:
				SetKeyboardProperty(KBPROPERTY_DELAY_BEFORE_REPEAT,  GetKeyboardProperty(KBPROPERTY_DELAY_BEFORE_REPEAT_MAX)-1-GetScrollBarPos(pWindow, KEYBDP_REPEAT_DELAY));
				SetKeyboardProperty(KBPROPERTY_REPEAT_FREQUENCY,     GetKeyboardProperty(KBPROPERTY_REPEAT_FREQUENCY_MAX)   -1-GetScrollBarPos(pWindow, KEYBDP_REPEAT_CPS));
				FlushKeyboardProperties();
				break;
			default:
				DefaultWindowProc(pWindow, messageType, parm1, parm2);
				break;
		}
	}
#endif

enum {
	CONTPNL_LISTVIEW = 0x10,
	CONTPNL_MENUBAR  = 0xFE,
};

int state=0;
void KbSetLedStatus(uint8_t status);

void Cpl$WindowPopup(Window* pWindow, const char* newWindowTitle, int newWindowX, int newWindowY, int newWindowW, int newWindowH, WindowProc newWindowProc, int newFlags);
void CALLBACK Cpl$WndProc (Window* pWindow, int messageType, int parm1, int parm2)
{
	//int npp = GetNumPhysPages(), nfpp = GetNumFreePhysPages();
	switch (messageType)
	{
		case EVENT_CREATE: {
			#define START_X 20
			#define STEXT_X 60
			#define START_Y 40
			#define DIST_ITEMS 36
			// Add a label welcoming the user to NanoShell.
			Rectangle r;
			
			RECT(r, 0, 0, 0, 0);
			AddControl (pWindow, CONTROL_MENUBAR, r, NULL, CONTPNL_MENUBAR, 0, 0);
			
			// Add some testing elements to the menubar.  A comboID of zero means you're adding to the root.
			AddMenuBarItem (pWindow, CONTPNL_MENUBAR, 0, 1, "Settings");
			AddMenuBarItem (pWindow, CONTPNL_MENUBAR, 0, 2, "Help");
			AddMenuBarItem (pWindow, CONTPNL_MENUBAR, 1, 3, "Exit");
			AddMenuBarItem (pWindow, CONTPNL_MENUBAR, 2, 4, "About Control Panel...");
			
			// Add a icon list view.
			#define PADDING_AROUND_LISTVIEW 4
			#define TOP_PADDING             (TITLE_BAR_HEIGHT + 5)
			RECT(r, 
				/*X Coord*/ PADDING_AROUND_LISTVIEW, 
				/*Y Coord*/ PADDING_AROUND_LISTVIEW + TITLE_BAR_HEIGHT + TOP_PADDING, 
				/*X Size */ 400 - PADDING_AROUND_LISTVIEW * 2, 
				/*Y Size */ 260 - PADDING_AROUND_LISTVIEW * 2 - TITLE_BAR_HEIGHT - TOP_PADDING
			);
			AddControl(pWindow, CONTROL_ICONVIEW, r, NULL, CONTPNL_LISTVIEW, 0, 0);
			
			// Add list items:
			ResetList(pWindow, CONTPNL_LISTVIEW);
			AddElementToList(pWindow, CONTPNL_LISTVIEW, "Display",             ICON_ADAPTER);
			AddElementToList(pWindow, CONTPNL_LISTVIEW, "Keyboard",            ICON_KEYBOARD);
			AddElementToList(pWindow, CONTPNL_LISTVIEW, "Mouse",               ICON_MOUSE);
			AddElementToList(pWindow, CONTPNL_LISTVIEW, "Desktop",             ICON_DESKTOP);
			AddElementToList(pWindow, CONTPNL_LISTVIEW, "Launcher",            ICON_HOME);
			AddElementToList(pWindow, CONTPNL_LISTVIEW, "Environment Paths",   ICON_DIRECTIONS);
			AddElementToList(pWindow, CONTPNL_LISTVIEW, "Permissions",         ICON_RESTRICTED);
			AddElementToList(pWindow, CONTPNL_LISTVIEW, "Serial Port",         ICON_SERIAL);
			AddElementToList(pWindow, CONTPNL_LISTVIEW, "Download over Serial",ICON_BILLBOARD);
			AddElementToList(pWindow, CONTPNL_LISTVIEW, "Date and Time",       ICON_CLOCK);
			AddElementToList(pWindow, CONTPNL_LISTVIEW, "Password Lock",       ICON_LOCK);
			AddElementToList(pWindow, CONTPNL_LISTVIEW, "Terminal settings",   ICON_COMMAND);
			AddElementToList(pWindow, CONTPNL_LISTVIEW, "App Memory Limit",    ICON_RESMON);
			
			break;
		}
		case EVENT_COMMAND: {
			if (parm1 == CONTPNL_MENUBAR)
			{
				switch (parm2)
				{
					case 3:
						DestroyWindow(pWindow);
						break;
					case 4:
						LaunchVersion();
						break;
				}
			}
			else if (parm1 == CONTPNL_LISTVIEW)
			{
				switch (parm2)
				{
					//NOTE: These are just mock for now.
					case 0:
					{
						char buff[2048];
						sprintf (buff, 
							"Display: %s\n"
							"Driver Name: %s\n\n"
							"Screen Size: %d x %d\n\n"
							"Framebuffer map address: 0x%X",
							
							"Generic VESA VBE-capable device",
							"NanoShell Basic VBE Display Driver",
							GetScreenWidth(), GetScreenHeight(),
							g_mainScreenVBEData.m_framebuffer32
						);
						MessageBox(pWindow, buff, "Display adapter info", MB_OK | ICON_ADAPTER << 16);
						break;
					}
					#if KEYBD_POPUP_WINDOW
					case 1:
					{
						Cpl$WindowPopup(
							pWindow,
							"Keyboard",
							pWindow->m_rect.left + 50,
							pWindow->m_rect.top  + 50,
							KEYBD_POPUP_WIDTH,
							KEYBD_POPUP_HEITE,
							Cpl$KeybdPopupWndProc,
							WF_NOMINIMZ
						);
						
						break;
					}
					#else
					case 1:
					{
						char buff[2048];
						sprintf (buff, 
							"Keyboard: %s\n"
							"Driver Name: %s",
							
							"Generic 101/102 Key PS/2 Keyboard HID device",
							"NanoShell Basic PS/2 Keyboard Driver",
							GetScreenWidth(), GetScreenHeight()
						);
						MessageBox(pWindow, buff, "Keyboard info", MB_OK | ICON_KEYBOARD << 16);
						break;
					}
					#endif
					#if MOUSE_POPUP_WINDOW
					case 2:
					{
						Cpl$WindowPopup(
							pWindow,
							"Mouse",
							pWindow->m_rect.left + 50,
							pWindow->m_rect.top  + 50,
							MOUSE_POPUP_WIDTH,
							MOUSE_POPUP_HEITE,
							Cpl$MousePopupWndProc,
							WF_NOMINIMZ
						);
						
						break;
					}
					#endif
					#if DESKT_POPUP_WINDOW
					case 3:
					{
						Cpl$WindowPopup(
							pWindow,
							"Desktop",
							pWindow->m_rect.left + 50,
							pWindow->m_rect.top  + 50,
							DESKTOP_POPUP_WIDTH,
							DESKTOP_POPUP_HEITE,
							Cpl$DesktopPopupWndProc,
							WF_NOMINIMZ
						);
						
						break;
					}
					#endif
					default:
						MessageBox(pWindow, "Not Implemented!", "Control Panel", MB_OK | ICON_WARNING << 16);
						break;
				}
			}
			else
				LogMsg("Unknown command event.  Parm1: %d Parm2: %d", parm1, parm2);
			break;
		}
		default:
			DefaultWindowProc(pWindow, messageType, parm1, parm2);
	}
}

void ControlEntry(__attribute__((unused)) int arg)
{
	// create ourself a window:
	int ww = 400, wh = 260;
	int wx = 150, wy = 150;
	
	Window* pWindow = CreateWindow ("Control Panel", wx, wy, ww, wh, Cpl$WndProc, 0);//WF_NOCLOSE);
	pWindow->m_iconID = ICON_FOLDER_SETTINGS;
	
	if (!pWindow)
	{
		DebugLogMsg("Hey, the window couldn't be created. Why?");
		return;
	}
	
	// setup:
	//ShowWindow(pWindow);
	
	// event loop:
#if THREADING_ENABLED
	while (HandleMessages (pWindow));
#endif
}
