#include "WindowModule.h"

int main(void) {

	WMwindow window = createWindow("2DM Window", 500, 500, WINDOW_NORMAL);
	centerWindow(&window);
	setIcon(&window, "icon/icon.bmp");	//.ico in windows, .bmp in linux

	float alpha = 0.0f;

	while(window.open) {

		if(pollEvents(&window)) {
			switch(event.eventType) {
				case CLOSE:
					closeWindow(&window);
					break;
				case RESIZE:
					{
						updateWindowViewport(&window);
						Vec2i windowSize = getWindowSize(&window);
						printf("Window resized X:%d, Y:%d\n", windowSize.x, windowSize.y);
					}
					break;
				case KEYDOWN:
					puts("Key down");
					break;
				case MOUSE_LBUTTON_PRESSED:
					{
						Vec2i mousePosition = getMousePosition(&window);
						printf("Mouse X: %d, Mouse Y: %d\n", mousePosition.x, mousePosition.y);
					}
				break;
			}
		}

		if(isKeyPressed(A)) alpha -=1;
		if(isKeyPressed(D)) alpha +=1;

		//Draw
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glPushMatrix();
		glRotatef(alpha, 0.0f, 1.0f, 0.0f);
		glBegin(GL_TRIANGLES);
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex2f(0.0f, 0.75f);
		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex2f(-0.75f, -0.75f);
		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex2f(0.75f, -0.75f);
		glEnd();
		glPopMatrix();

		display(&window);

		delay(10);
	}
	closeWindow(&window);

	return 0;
}
