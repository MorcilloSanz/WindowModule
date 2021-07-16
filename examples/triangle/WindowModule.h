/*
    MIT License

	Copyright (c) 2021 MorcilloSanz

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.

	-Description-
    Create a Window with an OpenGL context and handle it events for Linux and Windows

	-Compiler-
    Windows MINGW32:
    gcc main.c WindowModule.c -static-libgcc -std=c11 -mwindows -lopengl32 -lglu32 -g3 -o windowModule

    Windows Visual Studio:
    Linker->Input->Additional Dependencies
        opengl32.lib;glu32.lib;

    Linux GNU GCC:
    gcc main.c WindowModule.c -lX11 -lGL -lGLU -o windowModule

	-Warning-
    NO RESIZABLE WINDOW:
    Windows: The window doesn't have the maximize button and it's not possible to resize it with the mouse.
    Linux: The window doesn't have the maximize button BUT it is possible to resize it with the mouse.

    ONLY CLOSE WINDOW:
    Windows: The window can only be closed with the close button.
    Linux: The window can only be closed with the close button but also, the user is able to resize it with the mouse.

    Windows:
    resizeWindow(&window, x, y, width, height);

    Linux:
    resizeWindow(&window, DM_NULL, DM_NULL, width, height);

    No FPS limitation (Windows resizing don't work correctly without FSP limitation)

	-Bugs-
    Bug (WINDOWS) while mouse resizing ONLY IF there's no fps limitation
    Bug -Windows 7- (In some computers) when closing the window!

    Dev: MorcilloSanz
    Email: amorcillosanz@gmail.com
    Twitter: @MorcilloSanz
    GitHub: https://github.com/MorcilloSanz
    Version: alpha 1.0

	-Thanks to-
    Levi Webb / jarcode-foss for Assign _NET_WM_ICON cardinal and XWMHints pixmap for X11 window icons
*/

#ifndef _WINDOW_MODULE
#define _WINDOW_MODULE

#if defined(_WIN32)
#define _WINAPI
#endif

#if defined(__linux__) || defined(__unix__) || defined(__unix) || defined(unix)
#define _X11
#endif

#if defined(_MSC_VER)
#define _VISUAL
#endif

#if defined(__MINGW32__) || defined(__MINGW64__)
#define _MINGW
#endif

#if defined(__GNUC__)
#define _GNU
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#if defined(_MINGW) || defined(_GNU)
#include <sys/time.h>
#endif

#if defined(_VISUAL)
#include <tchar.h>
#endif

#if defined(_WINAPI)
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#endif

#if defined(_X11)
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysymdef.h>
#include <X11/Xatom.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <dirent.h>
#include <ftw.h>
#include <ctype.h>
#include <fcntl.h>
#endif

#define WINDOW_CLASS_NAME "WindowClass"

#define COLOR_BITS 24
#define DEPTH_BITS 16

#define IDT_TIMER1 0
#define DEFAULT_REFRESH_RATE 60

#define BMP_HEADER_MAGIC 0x4D42
#define BMP_BITFIELDS 3
#define DM_NULL 0

#define MAX_KEY_CODE 222
#define UNDEFINED_KEY 1

#if defined(_VISUAL)
#define WINDOW_CLASS_NAME L"WindowClass"
#endif

#if defined(_X11)
#define TRUE 1
#define FALSE 0
#define _NET_WM_STATE_ADD 1
#endif

#if defined(_X11)
typedef int BOOL;
typedef const char* LPCSTR;
#endif

typedef unsigned char byte;

// Don't change order
enum Key {
	ESC = 0x1B,
	F1 = 0x11, F2 = 0x12, F3 = 0x13, F4 = 0x14, F5 = 0x15, F6 = 0x16, F7 = 0x17, F8 = 0x18, F9 = 0x19, F10 = 0x1A, F11 = 0x1B, F12 = 0x1C,
	PRINTSCRN = 0x2C, SCROLL_LOCK = 0x91, PAUSE = 0x13, GRAVE_ACCENT = 0xC0,
	K0 = 0x30, K1 = 0x31, K2 = 0x32, K3 = 0x33, K4 = 0x34, K5 = 0x35, K6 = 0x36, K7 = 0x37, K8 = 0x38, K9 = 0x39,
	MINUS = 0x6D, EQUALS = UNDEFINED_KEY,
	BACKSPACE = 0x08, INSERT = 0x2D, HOME = UNDEFINED_KEY, PAGE_UP = UNDEFINED_KEY, LOCK = UNDEFINED_KEY, NSLASH = UNDEFINED_KEY, NASTERISK = UNDEFINED_KEY, NMINUS = UNDEFINED_KEY,
	TAB = 0x09,
	Q = 0x51, W = 0x57, E = 0x45, R = 0x52, T = 0x54, Y = 0x59, U = 0x55, I = 0x49, O = 0x4f, P = 0x50,
	LEFT_BRACKET = 0xDB, RIGHT_BRACKET = 0xDD, RETURN = 0x0D, DEL = 0x2E, END = UNDEFINED_KEY, PAGE_DOWN = UNDEFINED_KEY,
	N7 = 0x67, N8 = 0x68, N9 = 0x69, NPLUS = UNDEFINED_KEY, CAPS_LOCK = 0X14,
	A = 0x41, S = 0x53, D = 0x44, F = 0x46, G = 0x47, H = 0x48, J = 0x4A, K = 0x4B, L = 0x4C,
	SEMICOLON = 0xBA, APOSTROPHE = UNDEFINED_KEY,
	N4 = 0x64, N5 = 0x65, N6 = 0x66,
	SHIFT_LEFT = UNDEFINED_KEY, INTERNATIONAL = UNDEFINED_KEY,
	Z = 0x5A, X = 0x58, C = 0x43, V = 0x56, B = 0x42, N = 0x4E, M = 0x4D,
	COMMA, PERIOD, SLASH = UNDEFINED_KEY, SHIFT_RIGHT = UNDEFINED_KEY, BACKSLASH, CURSOR_UP = 0x26,
	N1 = 0x60, N2 = 0x61, N3 = 0x62, NENTER = UNDEFINED_KEY,
	CONTROL_LEFT = UNDEFINED_KEY, LOGO_LEFT = UNDEFINED_KEY, ALT_LEFT = UNDEFINED_KEY, SPACE = 0x20, ALT_RIGHT = UNDEFINED_KEY, LOGO_RIGHT = UNDEFINED_KEY, MENU = UNDEFINED_KEY, CONTROL_RIGHT = UNDEFINED_KEY,
	CURSOR_LEFT = 0x25, CURSOR_DOWN = 0x28, CURSOR_RIGHT = 0x27,
	N0 = UNDEFINED_KEY, NPERIOD = UNDEFINED_KEY
};
typedef enum Key Key;

enum EventType {
	NONE,
    // Window events
	CLOSE, MOVE, MOVING, RESIZE, OVER_BORDER,
    // Key events
	KEYDOWN, KEYUP, SYSTEM_KEYDOWN, SYSTEM_KEYUP,
    // Mouse events
	MOUSE_LBUTTON_PRESSED, MOUSE_LBUTTON_DBCLICK, MOUSE_LBUTTON_UP, 
    MOUSE_RBUTTON_PRESSED, MOUSE_RBUTTON_DBCLICK, MOUSE_RBUTTON_UP, 
    MOUSE_MBUTTON_PRESSED, MOUSE_MBUTTON_DBCLICK, MOUSE_MBUTTON_UP, 
    MOUSE_MOVE, MOUSE_WHEEL_UP, MOUSE_WHEEL_DOWN
};
typedef enum EventType EventType;

#if defined (_WINAPI)
struct WindowsEventProp {
    WPARAM wParam;
	LPARAM lParam;
	UINT message;
	EventType eventType;
};
typedef struct WindowsEventProp WindowsEventProp;
#endif

#if defined (_X11)
struct LinuxEventProp {
	XEvent xev;
};
typedef struct LinuxEventProp LinuxEventProp;
#endif

#if defined(_X11)
static long eventMask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | 
ButtonReleaseMask | VisibilityChangeMask | StructureNotifyMask | PropertyChangeMask;
#endif

struct Event {
#if defined (_WINAPI)
	WindowsEventProp eventProperties;
#endif
#if defined (_X11)
	LinuxEventProp eventProperties;
#endif
	EventType eventType;
};
typedef struct Event Event;

enum WindowType {
	WINDOW_NORMAL,
	WINDOW_ONLYCLOSE,
	WINDOW_NORESIZABLE,
	WINDOW_FULLSCREEN
};
typedef enum WindowType WindowType;

#if defined(_WINAPI)
struct WindowsWindowProp {
	HINSTANCE hInstance;
	LPSTR lpCmdLine;
	int nCmdShow;
	WNDCLASSEX wc;
	HWND hwnd;
	HDC hdc;
	HGLRC hrc;
	MSG msg;
	HICON hicon;
	HICON hiconBig;
#if defined(_VISUAL)
	WCHAR* wtitle;
#endif
};
typedef struct WindowsWindowProp WindowsWindowProp;
#endif

#if defined(_X11)
struct LinuxWindowProp {
	Display* dpy;
	Window root;
	XVisualInfo* vi;
	Colormap cmap;
	XSetWindowAttributes swa;
	Window win;
	GLXContext glc;
	XWindowAttributes xwa;
	XEvent xev;
};
typedef struct LinuxWindowProp LinuxWindowProp;
#endif

struct WMwindow {
#if defined(_WINAPI)
	WindowsWindowProp windowProperties;
#endif
#if defined(_X11)
	LinuxWindowProp windowProperties;
#endif
	unsigned int width;
	unsigned int height;
	const char* title;
	WindowType windowType;
	uint8_t open;
	uint8_t contextOpengl;
};
typedef struct WMwindow WMwindow;

#if defined(_X11)
struct MwmHints {
	unsigned long flags;
	unsigned long functions;
	unsigned long decorations;
	long input_mode;
	unsigned long status;
};
typedef struct MwmHints MwmHints;

enum {
	MWM_HINTS_FUNCTIONS = (1L << 0),
	MWM_HINTS_DECORATIONS = (1L << 1),
	MWM_FUNC_ALL = (1L << 0),
	MWM_FUNC_RESIZE = (1L << 1),
	MWM_FUNC_MOVE = (1L << 2),
	MWM_FUNC_MINIMIZE = (1L << 3),
	MWM_FUNC_MAXIMIZE = (1L << 4),
	MWM_FUNC_CLOSE = (1L << 5)
};
#endif

#if defined(_GNU)
struct __attribute__((packed)) BmpHeader {
	uint16_t header;
	uint32_t size;
	uint16_t reserved0, reserved1;
	uint32_t offset;
	/* BITMAPINFOHEADER */
	uint32_t header_size, width, height;
	uint16_t planes, bits_per_pixel;
	uint32_t compression, image_size, hres, vres, colors, colors_used;
};
#endif

struct TimeManagement {
	long previousTime;
	long currentTime;
	byte timeStarted;
	int xpending;	//Events remaining
};
typedef struct TimeManagement TimeManagement;

struct Vec2i {
    int x;
    int y;
};
typedef struct Vec2i Vec2i;

struct Vec2f {
    float x;
    float y;
};
typedef struct Vec2f Vec2f;

extern unsigned int keys[];
extern unsigned char keyCodes[];
extern unsigned int keysBuffer[MAX_KEY_CODE];
extern Event event;
extern TimeManagement timeManagement;

#if defined(_WINAPI)
// Mouse functions
Vec2i getDesktopMousePosition();
void mouseResizing(WMwindow* window);

// Debug functions
void setDebugColor(unsigned int color);
void createConsole();
void hideConsole();
void showConsole();

// Event callback
static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
#endif

#if defined(_VISUAL)
WCHAR* toWCHAR(const char* str);
#endif

#if defined(_X11)
static int WndProc(WMwindow* window);
#endif

// Time functions
long long getMillis(void);
void delay(unsigned int ms);
void sysDelay(unsigned int ms);

// Event functions
int pollEvents(WMwindow* window);
char ASCIItoKeyCode(char key);
uint8_t isKeyPressed(char key);
Vec2i getMousePosition(WMwindow* window);

// OpenGL functions
void enableOpenGL(WMwindow* window);
void disableOpenGL(WMwindow* window);
void display(WMwindow* window);

// Get window information
Vec2i getWindowSize(WMwindow* window);
Vec2i getWindowPosition(WMwindow* window);
Vec2i getDesktopResolution();

// Window functions
void updateWindowViewport(WMwindow* window);
void centerWindow(WMwindow* window);
void moveWindow(WMwindow* window, unsigned int x, unsigned int y);
void resizeWindow(WMwindow* window, unsigned int x, unsigned int y, unsigned int width, unsigned int height);
void minimizeWindow(WMwindow* window);
void maximizeWindow(WMwindow* window);
BOOL enterFullscreen(WMwindow* window);
int setIcon(WMwindow* window, const char* iconPath);
int setTitle(WMwindow* window, const char* title);
void closeWindow(WMwindow* window);
WMwindow createWindow(const char* title, unsigned int width, unsigned int height, WindowType windowType);

#endif