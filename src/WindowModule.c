#include "WindowModule.h"

// Don't change order
unsigned int keys[] = {
	ESC, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,  PRINTSCRN, 
    SCROLL_LOCK, PAUSE, GRAVE_ACCENT, K1, K2, K3, K4, K5, K6, K7, K8, K9, 
    K0, MINUS, EQUALS, BACKSPACE, INSERT, HOME, PAGE_UP, LOCK, NSLASH, NASTERISK, 
    NMINUS, TAB, Q, W, E, R, T, Y, U, I, O, P, LEFT_BRACKET, RIGHT_BRACKET, RETURN, 
    DEL, END, PAGE_DOWN, N7, N8, N9, NPLUS, CAPS_LOCK, A, S, D, F, G, H, J, K, L, 
    SEMICOLON, APOSTROPHE , N4, N5, N6, SHIFT_LEFT, INTERNATIONAL, Z, X, C, V, B, 
    N, M, COMMA, PERIOD, SLASH, SHIFT_RIGHT, BACKSLASH, CURSOR_UP, N1, N2, N3, NENTER, 
    CONTROL_LEFT, LOGO_LEFT, ALT_LEFT, SPACE, ALT_RIGHT, LOGO_RIGHT, MENU, CONTROL_RIGHT, 
    CURSOR_LEFT, CURSOR_DOWN, CURSOR_RIGHT, N0, NPERIOD
};

// Don't change order
unsigned char keyCodes[] = {
	9, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 95, 96, 111, 78, 110, 49, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 106, 97, 99, 77, 112, 63, 82, 23, 24, 25, 26, 27, 28, 29, 30, 31, 
    32, 33, 34, 35, 36, 107, 103, 105, 79, 80, 81, 86, 66, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 
    48, 83, 84, 85, 50, 94, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 51, 98, 87, 88, 89, 108, 37, 
    115, 64, 65, 113, 116, 117, 109, 100, 104, 102, 90, 91
};

unsigned int keysBuffer[MAX_KEY_CODE];

Event event;

TimeManagement timeManagement;

long long getMillis(void) {
#if defined(_MINGW) || defined(_GNU)
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (((long long)tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
#endif
#if defined(_VISUAL)
	return GetTickCount();
#endif
	return 0;
}

// Sleeps the main thread using the TimeManagement struct
void delay(unsigned int ms) {
	if (!timeManagement.timeStarted) {
		timeManagement.previousTime = getMillis();
		timeManagement.timeStarted = TRUE;
	}
	BOOL standBy = TRUE;
	while (standBy) {
		timeManagement.currentTime = getMillis();
		if ((timeManagement.currentTime - timeManagement.previousTime) >= ms) {
			timeManagement.previousTime = timeManagement.currentTime;
			standBy = FALSE;
		}
	}
}

// Sleeps the main thread using Sleep and usleep functions.
void sysDelay(unsigned int ms) {
#if defined(_WINAPI)
	Sleep(ms);
#endif
#if defined(_X11)
	if (timeManagement.xpending == 0)
		usleep(ms * 1000);
#endif
}

int pollEvents(WMwindow* window) {
#if defined(_WINAPI)
	int got = PeekMessage(&window->windowProperties.msg, NULL, 0, 0, PM_REMOVE);
	TranslateMessage(&window->windowProperties.msg);
	DispatchMessage(&window->windowProperties.msg);
	return got;
#endif
#if defined(_X11)
	int got;
	timeManagement.xpending = XPending(window->windowProperties.dpy);
	if (timeManagement.xpending > 0) {
		XNextEvent(window->windowProperties.dpy, &event.eventProperties.xev);
		got = WndProc(window);
	}
	return got;
#endif
}

char ASCIItoKeyCode(char key) {
	for (int i = 0; i < MAX_KEY_CODE; i++)
		if (key == keys[i])
			return keyCodes[i];
	return -1;
}

uint8_t isKeyPressed(char key) {
#if defined(_WINAPI)
	return (GetAsyncKeyState(key) < 0) ? TRUE : FALSE;
#endif
#if defined(_X11)
	char keyCode = ASCIItoKeyCode(key);
	return (keysBuffer[keyCode] == TRUE) ? TRUE : FALSE;
#endif
}

Vec2i getMousePosition(WMwindow* window) {
	Vec2i pos;
#if defined(_WINAPI)
	POINT position;
	GetCursorPos(&position);
	ScreenToClient(window->windowProperties.hwnd, &position);
	pos.x = position.x;
	pos.y = position.y;
#endif
#if defined(_X11)
	pos.x = event.eventProperties.xev.xbutton.x;
	pos.y = event.eventProperties.xev.xbutton.y;
#endif
	return pos;
}

void enableOpenGL(WMwindow* window) {
#if defined(_WINAPI)
	PIXELFORMATDESCRIPTOR pfd;
	int iFormat;
	window->windowProperties.hdc = GetDC(window->windowProperties.hwnd);
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = COLOR_BITS;
	pfd.cDepthBits = DEPTH_BITS;
	pfd.iLayerType = PFD_MAIN_PLANE;
	iFormat = ChoosePixelFormat(window->windowProperties.hdc, &pfd);
	SetPixelFormat(window->windowProperties.hdc, iFormat, &pfd);
	window->windowProperties.hrc = wglCreateContext(window->windowProperties.hdc);
	wglMakeCurrent(window->windowProperties.hdc, window->windowProperties.hrc);
#endif
#if defined(_X11)
	window->windowProperties.glc = glXCreateContext(window->windowProperties.dpy, window->windowProperties.vi, NULL, GL_TRUE);
	glXMakeCurrent(window->windowProperties.dpy, window->windowProperties.win, window->windowProperties.glc);
#endif
	window->contextOpengl = 1;
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
}

void disableOpenGL(WMwindow* window) {
#if defined(_WINAPI)
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(window->windowProperties.hrc);
	ReleaseDC(window->windowProperties.hwnd, window->windowProperties.hdc);
#endif
#if defined(_X11)
	glXMakeCurrent(window->windowProperties.dpy, None, NULL);
	glXDestroyContext(window->windowProperties.dpy, window->windowProperties.glc);
#endif
}

void display(WMwindow* window) {
#if defined(_WINAPI)
	SwapBuffers(window->windowProperties.hdc);
#endif
#if defined(_X11)
	glXSwapBuffers(window->windowProperties.dpy, window->windowProperties.win);
#endif
}

Vec2i getWindowSize(WMwindow* window) {
	Vec2i size;
#if defined(_WINAPI)
	RECT rect;
	GetWindowRect(window->windowProperties.hwnd, &rect);
	size.x = rect.right - rect.left;
	size.y = rect.bottom - rect.top;
#endif
#if defined(_X11)
	XWindowAttributes xwa;
	XGetWindowAttributes(window->windowProperties.dpy, window->windowProperties.win, &xwa);
	size.x = xwa.width;
	size.y = xwa.height;
#endif
	return size;
}

Vec2i getWindowPosition(WMwindow* window) {
	Vec2i position;
#if defined(_WINAPI)
	RECT rect;
	GetWindowRect(window->windowProperties.hwnd, &rect);
	position.x = rect.left;
	position.y = rect.top;
#endif
#if defined(_X11)
	XWindowAttributes xwa;
	XGetWindowAttributes(window->windowProperties.dpy, window->windowProperties.win, &xwa);
	position.x = xwa.x;
	position.y = xwa.y;
#endif
	return position;
}

Vec2i getDesktopResolution() {
	Vec2i resolution;
#if defined(_WINAPI)
	RECT desktop;
	const HWND hDesktop = GetDesktopWindow();
	GetWindowRect(hDesktop, &desktop);
	resolution.x = desktop.right;
	resolution.y = desktop.bottom;
#endif
#if defined(_X11)
	Display* d = XOpenDisplay(NULL);
	Screen* s = DefaultScreenOfDisplay(d);
	resolution.x = s->width;
	resolution.y = s->height;
#endif
	return resolution;
}

void updateWindowViewport(WMwindow* window) {
#if defined(_WINAPI)
	RECT rect;
	GetClientRect(window->windowProperties.hwnd, &rect);
	glViewport(0, 0, rect.right, rect.bottom);
#endif
#if defined(_X11)
	Vec2i windowSize = getWindowSize(window);
	glViewport(0, 0, windowSize.x, windowSize.y);
#endif
}

void centerWindow(WMwindow* window) {
	if (window->windowType != WINDOW_FULLSCREEN) {
		Vec2i resolution = getDesktopResolution();
		unsigned int newX = resolution.x / 2 - window->width / 2;
		unsigned int newY = resolution.y / 2 - window->height / 2;
#if defined(_WINAPI)
		MoveWindow(window->windowProperties.hwnd, newX, newY, window->width, window->height, FALSE);
#endif
#if defined(_X11)
		XMoveWindow(window->windowProperties.dpy, window->windowProperties.win, newX, newY);
#endif
	}
}

void moveWindow(WMwindow* window, unsigned int x, unsigned int y) {
#if defined(_WINAPI)
	VEC2I windowSize = getWindowSize(window);
	MoveWindow(window->windowProperties.hwnd, x, y, windowSize.x, windowSize.y, FALSE);
#endif
#if defined(_X11)
	XMoveWindow(window->windowProperties.dpy, window->windowProperties.win, x, y);
#endif
}

void resizeWindow(WMwindow* window, unsigned int x, unsigned int y, unsigned int width, unsigned int height) {
#if defined(_WINAPI)
	MoveWindow(window->windowProperties.hwnd, x, y, width, height, FALSE);
#endif
#if defined(_X11)
	XResizeWindow(window->windowProperties.dpy, window->windowProperties.win, width, height);
#endif
}

void minimizeWindow(WMwindow* window) {
#if defined(_WINAPI)
	ShowWindow(window->windowProperties.hwnd, SW_MINIMIZE);
#endif
#if defined(_X11)
	XIconifyWindow(window->windowProperties.dpy, window->windowProperties.win, DefaultScreen(window->windowProperties.dpy));
#endif
}

void maximizeWindow(WMwindow* window) {
#if defined(_WINAPI)
	ShowWindow(window->windowProperties.hwnd, SW_MAXIMIZE);
#endif
#if defined(_X11)
	XEvent xev;
	Atom wm_state = XInternAtom(window->windowProperties.dpy, "_NET_WM_STATE", False);
	Atom max_horz = XInternAtom(window->windowProperties.dpy, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
	Atom max_vert = XInternAtom(window->windowProperties.dpy, "_NET_WM_STATE_MAXIMIZED_VERT", False);
	memset(&xev, 0, sizeof(xev));
	xev.type = ClientMessage;
	xev.xclient.window = window->windowProperties.win;
	xev.xclient.message_type = wm_state;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = _NET_WM_STATE_ADD;
	xev.xclient.data.l[1] = max_horz;
	xev.xclient.data.l[2] = max_vert;
	XSendEvent(window->windowProperties.dpy, DefaultRootWindow(window->windowProperties.dpy), False, SubstructureNotifyMask, &xev);
#endif
}

BOOL enterFullscreen(WMwindow* window) {
	BOOL isChangeSuccessful;
#if defined(_WINAPI)
	DEVMODE fullscreenSettings;
	RECT windowBoundary;
	VEC2I resolution = getDesktopResolution();

	EnumDisplaySettings(NULL, 0, &fullscreenSettings);
	fullscreenSettings.dmPelsWidth = resolution.x;
	fullscreenSettings.dmPelsHeight = resolution.y;
	fullscreenSettings.dmBitsPerPel = COLOR_BITS;
	fullscreenSettings.dmDisplayFrequency = DEFAULT_REFRESH_RATE;
	fullscreenSettings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;

	SetWindowLongPtr(window->windowProperties.hwnd, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST);
	SetWindowLongPtr(window->windowProperties.hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
	SetWindowPos(window->windowProperties.hwnd, HWND_TOPMOST, 0, 0, resolution.x, resolution.y, SWP_SHOWWINDOW);
	isChangeSuccessful = ChangeDisplaySettings(&fullscreenSettings, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL;
	ShowWindow(window->windowProperties.hwnd, SW_MAXIMIZE);
#endif
#if defined(_X11)
	XEvent xev;
	Atom wm_state = XInternAtom(window->windowProperties.dpy, "_NET_WM_STATE", False);
	Atom fullscreen = XInternAtom(window->windowProperties.dpy, "_NET_WM_STATE_FULLSCREEN", False);
	memset(&xev, 0, sizeof(xev));

	xev.type = ClientMessage;
	xev.xclient.window = window->windowProperties.win;
	xev.xclient.message_type = wm_state;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = 1;
	xev.xclient.data.l[1] = fullscreen;
	xev.xclient.data.l[2] = 0;

	XSendEvent(window->windowProperties.dpy, DefaultRootWindow(window->windowProperties.dpy), False, SubstructureNotifyMask | SubstructureRedirectMask, &xev);
#endif
	return isChangeSuccessful;
}

int setIcon(WMwindow* window, const char* iconPath) {
#if defined(_WINAPI) && defined(_MINGW)
	window->windowProperties.hicon = (HICON)LoadImage(NULL, iconPath, IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
	window->windowProperties.hiconBig = (HICON)LoadImage(NULL, iconPath, IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
	SendMessage(window->windowProperties.hwnd, WM_SETICON, ICON_SMALL, (LPARAM)window->windowProperties.hicon);
	SendMessage(window->windowProperties.hwnd, WM_SETICON, ICON_BIG, (LPARAM)window->windowProperties.hiconBig);
	if (window->windowProperties.hicon == NULL || window->windowProperties.hiconBig == NULL)
		return -1;
#endif
#if defined(_WINAPI) && defined(_VISUAL)
	WCHAR* wiconPath = toWCHAR(iconPath);
	window->windowProperties.hicon = (HICON)LoadImageW(NULL, wiconPath, IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
	window->windowProperties.hiconBig = (HICON)LoadImageW(NULL, wiconPath, IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
	SendMessage(window->windowProperties.hwnd, WM_SETICON, ICON_SMALL, (LPARAM)window->windowProperties.hicon);
	SendMessage(window->windowProperties.hwnd, WM_SETICON, ICON_BIG, (LPARAM)window->windowProperties.hiconBig);
	free(wiconPath);
	if (window->windowProperties.hicon == NULL || window->windowProperties.hiconBig == NULL)
		return -1;
#endif
#if defined(_X11)
	int fd = open(iconPath, O_RDONLY);
	if (fd == -1) {
		fprintf(stderr, "failed to load icon '%s': %s\n", iconPath, strerror(errno));
		return -1;
	}
	struct stat st;
	fstat(fd, &st);
	const struct BmpHeader* header = (struct BmpHeader*)mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (header->header != BMP_HEADER_MAGIC) {
		fprintf(stderr, "failed to load icon '%s': invalid BMP header.\n", iconPath);
		close(fd);
		return -1;
	}
	if (header->bits_per_pixel != 32) {
		fprintf(stderr, "failed to load icon '%s': wrong bit depth (%d).\n",
			iconPath, (int)header->bits_per_pixel);
		close(fd);
		return -1;
	}
	if (header->planes != 1 || header->compression != BMP_BITFIELDS) {
		fprintf(stderr, "failed to load icon '%s': invalid BMP format, requires RGBA bitfields.\n", iconPath);
		close(fd);
		return -1;
	}

	const char* data = (const char*)(((const uint8_t*)header) + header->offset);

	XWMHints hints = {};
	hints.flags = IconPixmapHint;
	hints.icon_pixmap = XCreateBitmapFromData(window->windowProperties.dpy, window->windowProperties.win, data, header->width, header->height);
	XSetWMHints(window->windowProperties.dpy, window->windowProperties.win, &hints);

	size_t sz = header->width * header->height;
	size_t asz = sz + 2;
	unsigned long* off = (unsigned long*)malloc(asz * sizeof(unsigned long));
	for (size_t x = 0; x < header->width; ++x) {
		for (size_t y = 0; y < header->height; ++y) {
			off[x + (((header->height - 1) - y) * header->height) + 2]
				= ((const uint32_t*)data)[x + (y * header->height)];
		}
	}

	off[0] = header->width;
	off[1] = header->height;
	XChangeProperty(window->windowProperties.dpy, window->windowProperties.win, XInternAtom(window->windowProperties.dpy, "_NET_WM_ICON", True),
		XA_CARDINAL, 32, PropModeReplace, (const unsigned char*)off, asz);
	free(off);
	close(fd);
#endif
	return 1;
}

int setTitle(WMwindow* window, const char* title) {
#if defined(_WINAPI)
	SetWindowTextA(window->windowProperties.hwnd, title);
#endif
#if defined(_X11)
	XStoreName(window->windowProperties.dpy, window->windowProperties.win, title);
	XClassHint* class_hint = XAllocClassHint();
	if (class_hint) {
		class_hint->res_name = class_hint->res_class = (char*)title;
		XSetClassHint(window->windowProperties.dpy, window->windowProperties.win, class_hint);
		XFree(class_hint);
	}
#endif
	return 1;
}

void closeWindow(WMwindow* window) {
#if defined(_WINAPI)
	window->open = FALSE;
	disableOpenGL(window);
	PostQuitMessage(0);
#endif
#if defined(_X11)
	window->open = FALSE;
	disableOpenGL(window);
	XDestroyWindow(window->windowProperties.dpy, window->windowProperties.win);
	XCloseDisplay(window->windowProperties.dpy);
	exit(0);
#endif
}

WMwindow createWindow(const char* title, unsigned int width, unsigned int height, WindowType windowType) {
	WMwindow window;
#if defined(_WINAPI) && defined(_MINGW)
	window.windowProperties.hInstance = GetModuleHandle(0);
	memset(&window.windowProperties.wc, 0, sizeof(window.windowProperties.wc));
	window.windowProperties.wc.cbSize = sizeof(WNDCLASSEX);
	window.windowProperties.wc.lpfnWndProc = &WndProc;
	window.windowProperties.wc.hInstance = window.windowProperties.hInstance;
	window.windowProperties.wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	window.windowProperties.wc.hbrBackground = NULL;
	window.windowProperties.wc.lpszClassName = WINDOW_CLASS_NAME;
	window.windowProperties.wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	window.windowProperties.wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&window.windowProperties.wc))
		MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

	VEC2I resolution = getDesktopResolution();

	switch (windowType) {
	case WINDOW_NORMAL:
		window.windowProperties.hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, WINDOW_CLASS_NAME, title, WS_VISIBLE | WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, window.windowProperties.hInstance, NULL);
		window.windowType = WINDOW_NORMAL;
		break;
	case WINDOW_NORESIZABLE:
		window.windowProperties.hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, WINDOW_CLASS_NAME, title, WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU | WS_VISIBLE,
			CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, window.windowProperties.hInstance, NULL);
		window.windowType = WINDOW_NORESIZABLE;
		break;
	case WINDOW_FULLSCREEN:
		window.windowProperties.hwnd = CreateWindow(WINDOW_CLASS_NAME, title, WS_BORDER, 0, 0, resolution.x, resolution.y, NULL, NULL, window.windowProperties.hInstance, NULL);
		SetWindowLong(window.windowProperties.hwnd, GWL_STYLE, 0);
		ShowWindow(window.windowProperties.hwnd, SW_SHOW);
		window.windowType = WINDOW_FULLSCREEN;
		break;
	case WINDOW_ONLYCLOSE:
		window.windowProperties.hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, WINDOW_CLASS_NAME, title, WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE,
			CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, window.windowProperties.hInstance, NULL);
		window.windowType = WINDOW_ONLYCLOSE;
		break;
	default:
		window.windowProperties.hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, WINDOW_CLASS_NAME, title, WS_VISIBLE | WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, window.windowProperties.hInstance, NULL);
		window.windowType = WINDOW_NORMAL;
		break;
	}
#endif
#if defined(_WINAPI) && defined(_VISUAL)

	window.windowProperties.wtitle = toWCHAR(title);

	window.windowProperties.hInstance = GetModuleHandle(0);
	memset(&window.windowProperties.wc, 0, sizeof(window.windowProperties.wc));
	window.windowProperties.wc.cbSize = sizeof(WNDCLASSEX);
	window.windowProperties.wc.lpfnWndProc = &WndProc;
	window.windowProperties.wc.hInstance = window.windowProperties.hInstance;
	window.windowProperties.wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	window.windowProperties.wc.hbrBackground = NULL;
	window.windowProperties.wc.lpszClassName = WINDOW_CLASS_NAME;
	window.windowProperties.wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	window.windowProperties.wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&window.windowProperties.wc))
		MessageBox(NULL, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);

	VEC2I resolution = getDesktopResolution();

	switch (windowType) {
	case WINDOW_NORMAL:
		window.windowProperties.hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, WINDOW_CLASS_NAME, window.windowProperties.wtitle, WS_VISIBLE | WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, window.windowProperties.hInstance, NULL);
		window.windowType = WINDOW_NORMAL;
		break;
	case WINDOW_NORESIZABLE:
		window.windowProperties.hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, WINDOW_CLASS_NAME, window.windowProperties.wtitle, WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU | WS_VISIBLE,
			CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, window.windowProperties.hInstance, NULL);
		window.windowType = WINDOW_NORESIZABLE;
		break;
	case WINDOW_FULLSCREEN:
		window.windowProperties.hwnd = CreateWindow(WINDOW_CLASS_NAME, window.windowProperties.wtitle, WS_BORDER, 0, 0, resolution.x, resolution.y, NULL, NULL, window.windowProperties.hInstance, NULL);
		SetWindowLong(window.windowProperties.hwnd, GWL_STYLE, 0);
		ShowWindow(window.windowProperties.hwnd, SW_SHOW);
		window.windowType = WINDOW_FULLSCREEN;
		break;
	case WINDOW_ONLYCLOSE:
		window.windowProperties.hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, WINDOW_CLASS_NAME, window.windowProperties.wtitle, WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE,
			CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, window.windowProperties.hInstance, NULL);
		window.windowType = WINDOW_ONLYCLOSE;
		break;
	default:
		window.windowProperties.hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, WINDOW_CLASS_NAME, window.windowProperties.wtitle, WS_VISIBLE | WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, window.windowProperties.hInstance, NULL);
		window.windowType = WINDOW_NORMAL;
		break;
	}

#endif
#if defined(_X11)

	window.windowProperties.dpy = XOpenDisplay(NULL);
	if (window.windowProperties.dpy == NULL) {
		printf("\n\tcannot connect to X server\n\n");
		exit(0);
	}

	window.windowProperties.root = DefaultRootWindow(window.windowProperties.dpy);

	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	window.windowProperties.vi = glXChooseVisual(window.windowProperties.dpy, 0, att);
	if (window.windowProperties.vi == NULL) {
		printf("\n\tno appropriate visual found\n\n");
		exit(0);
	}
	else
		printf("\n\tvisual %p selected\n", (void*)window.windowProperties.vi->visualid); //%p creates hexadecimal output like in glxinfo


	window.windowProperties.swa.colormap = XCreateColormap(window.windowProperties.dpy, window.windowProperties.root, window.windowProperties.vi->visual, AllocNone);
	window.windowProperties.swa.event_mask = eventMask;
	XSelectInput(window.windowProperties.dpy, window.windowProperties.root, eventMask);

	window.windowProperties.win = XCreateWindow(window.windowProperties.dpy, window.windowProperties.root, 0, 0, width, height,
		0, window.windowProperties.vi->depth, InputOutput, window.windowProperties.vi->visual, CWColormap | CWEventMask, &window.windowProperties.swa);
	window.windowType = windowType;

	XAllowEvents(window.windowProperties.dpy, AsyncBoth, CurrentTime);

	// Window title hint
	XClassHint* class_hint = XAllocClassHint();
	if (class_hint) {
		class_hint->res_name = class_hint->res_class = (char*)title;
		XSetClassHint(window.windowProperties.dpy, window.windowProperties.win, class_hint);
		XFree(class_hint);
	}

	// Default logo
	unsigned long logoBuffer[] = { 0 };
	int logoLength = 1;
	Atom net_wm_icon = XInternAtom(window.windowProperties.dpy, "_NET_WM_ICON", False);
	Atom cardinal = XInternAtom(window.windowProperties.dpy, "CARDINAL", False);
	XChangeProperty(window.windowProperties.dpy, window.windowProperties.win, net_wm_icon, cardinal, 32, PropModeReplace, (const unsigned char*)logoBuffer, logoLength);

	XMapWindow(window.windowProperties.dpy, window.windowProperties.win);
	XStoreName(window.windowProperties.dpy, window.windowProperties.win, title);
	XFlush(window.windowProperties.dpy);

	switch (windowType) {
	case WINDOW_FULLSCREEN:
		enterFullscreen(&window);
		break;
	case WINDOW_NORESIZABLE:
	{
		struct MwmHints hints;
		Atom wm = XInternAtom(window.windowProperties.dpy, "_MOTIF_WM_HINTS", False);
		hints.functions = MWM_FUNC_MOVE | MWM_FUNC_MINIMIZE | MWM_FUNC_CLOSE;
		hints.flags = MWM_HINTS_FUNCTIONS;
		XChangeProperty(window.windowProperties.dpy, window.windowProperties.win, wm, XA_ATOM, 32, PropModeReplace, (unsigned char*)&hints, 5);
	}
	break;
	case WINDOW_ONLYCLOSE:
	{
		struct MwmHints hints;
		Atom wm = XInternAtom(window.windowProperties.dpy, "_MOTIF_WM_HINTS", False);
		hints.functions = MWM_FUNC_MOVE | MWM_FUNC_CLOSE;
		hints.flags = MWM_HINTS_FUNCTIONS;
		XChangeProperty(window.windowProperties.dpy, window.windowProperties.win, wm, XA_ATOM, 32, PropModeReplace, (unsigned char*)&hints, 5);
	}
	break;
	}
#endif

	window.title = title;
	window.width = width;
	window.height = height;
	window.open = TRUE;

	//Start timing
	timeManagement.previousTime = getMillis();
	timeManagement.timeStarted = TRUE;

	//Keys mapping
	memset(keysBuffer, 0, MAX_KEY_CODE);

	enableOpenGL(&window);

	return window;
}

#if defined(_WINAPI)
VEC2I getDesktopMousePosition() {
	VEC2I pos;
	POINT position;
	GetCursorPos(&position);
	pos.x = position.x;
	pos.y = position.y;
	return pos;
}

// Only use it if there's a FATAL bug when resizing with the mouse.
void mouseResizing(WINDOW* window) {
	updateWindowViewport(window);
	ShowWindow(window->windowProperties.hwnd, SW_HIDE);
	ShowWindow(window->windowProperties.hwnd, SW_SHOW);
}

void setDebugColor(unsigned int color) {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void createConsole() {
#if defined(_MINGW)
	AllocConsole();
	freopen("conin$", "r", stdin);
	freopen("conout$", "w", stdout);
	freopen("conout$", "w", stderr);
#endif
}

void hideConsole() {
	HWND console = GetConsoleWindow();
	ShowWindow(console, SW_HIDE);
}

void showConsole() {
	HWND console = GetConsoleWindow();
	ShowWindow(console, SW_SHOW);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	event.eventProperties.wParam = wParam;
	event.eventProperties.lParam = lParam;
	event.eventProperties.message = message;
	short wheel_up = 0;
	switch (message) {
	case WM_CLOSE:
		event.eventType = CLOSE;
		break;
	case WM_DESTROY:
		KillTimer(hWnd, IDT_TIMER1);
		event.eventType = CLOSE;
		break;
	case WM_QUIT:
		event.eventType = CLOSE;
		break;
	case WM_MOVE:
		event.eventType = MOVE;
		break;
	case WM_MOVING:
		event.eventType = MOVING;
		break;
	case WM_SIZE:
		event.eventType = RESIZE;
		break;
	case 0x232:	// Mouse Resizing
		event.eventType = RESIZE;
		break;
	case 0xa0:
		event.eventType = OVER_BORDER;
		break;
	case WM_TIMER:
		switch (wParam) {
		case IDT_TIMER1:
			return 0;
		}
		break;
	case WM_KEYDOWN:
		event.eventType = KEYDOWN;
		break;
	case WM_KEYUP:
		event.eventType = KEYUP;
		break;
	case WM_KILLFOCUS:
		HideCaret(hWnd);
		DestroyCaret();
		break;
	case WM_MOUSEMOVE:
		event.eventType = MOUSE_MOVE;
		break;
	case WM_LBUTTONDOWN:
		event.eventType = MOUSE_LBUTTON_PRESSED;
		break;
	case WM_LBUTTONUP:
		event.eventType = MOUSE_LBUTTON_UP;
		break;
	case WM_LBUTTONDBLCLK:
		event.eventType = MOUSE_LBUTTON_DBCLICK;
		break;
	case WM_RBUTTONDOWN:
		event.eventType = MOUSE_RBUTTON_PRESSED;
		break;
	case WM_RBUTTONUP:
		event.eventType = MOUSE_RBUTTON_UP;
		break;
	case WM_RBUTTONDBLCLK:
		event.eventType = MOUSE_RBUTTON_DBCLICK;
		break;
	case WM_MBUTTONDOWN:
		event.eventType = MOUSE_MBUTTON_PRESSED;
		break;
	case WM_MBUTTONUP:
		event.eventType = MOUSE_MBUTTON_UP;
		break;
	case WM_MBUTTONDBLCLK:
		event.eventType = MOUSE_MBUTTON_DBCLICK;
		break;
	case WM_MOUSEWHEEL:
		wheel_up = GET_WHEEL_DELTA_WPARAM(wParam);
		event.eventType = (wheel_up > 0) ? MOUSE_WHEEL_UP : MOUSE_WHEEL_DOWN;
		break;
	default:
		event.eventType = NONE;
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}
#endif

#if defined(_VISUAL)
WCHAR* toWCHAR(const char* str) {
	const unsigned int length = strlen(str);
	WCHAR* buffer = (WCHAR*)malloc(sizeof(WCHAR) * length);
	int nChars = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
	MultiByteToWideChar(CP_ACP, 0, str, -1, (LPWSTR)buffer, nChars);
	return buffer;
}
#endif

#if defined(_X11)
int WndProc(WMwindow* window) {
	switch (event.eventProperties.xev.type) {
	case Expose:
		return 1;
	case ConfigureNotify:
	{
		XConfigureEvent xce = event.eventProperties.xev.xconfigure;
		if (xce.width != window->width || xce.height != window->height) {
			window->width = xce.width;
			window->height = xce.height;
			event.eventType = RESIZE;
		}
	}
	return 1;
	case KeyPress:
	{
		keysBuffer[event.eventProperties.xev.xkey.keycode] = TRUE;
		event.eventType = KEYDOWN;
	}
	return 1;
	case KeyRelease:
	{
		keysBuffer[event.eventProperties.xev.xkey.keycode] = FALSE;
		event.eventType = KEYUP;
	}
	return 1;
	/*
		1 = left button
		2 = middle button (pressing the scroll wheel)
		3 = right button
		4 = turn scroll wheel up
		5 = turn scroll wheel down
		6 = push scroll wheel left
		7 = push scroll wheel right
		8 = 4th button (aka browser backward button)
		9 = 5th button (aka browser forward button)
	*/
	case ButtonPress:
		switch (event.eventProperties.xev.xbutton.button) {
		case Button1:
			event.eventType = MOUSE_LBUTTON_PRESSED;
			return 1;
		case Button2:
			event.eventType = MOUSE_MBUTTON_PRESSED;
			return 1;
		case Button3:
			event.eventType = MOUSE_RBUTTON_PRESSED;
			return 1;
		case Button4:
			event.eventType = MOUSE_WHEEL_UP;
			return 1;
		case Button5:
			event.eventType = MOUSE_WHEEL_DOWN;
			return 1;
		}
		return 1;
	case ButtonRelease:
		switch (event.eventProperties.xev.xbutton.button) {
		case Button1:
			event.eventType = MOUSE_LBUTTON_UP;
			return 1;
		case Button2:
			event.eventType = MOUSE_MBUTTON_UP;
			return 1;
		case Button3:
			event.eventType = MOUSE_RBUTTON_UP;
			return 1;
		}
		return 1;
	default:
		event.eventType = NONE;
		return 0;
	}
	return 0;
}
#endif