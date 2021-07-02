![alt text](https://github.com/MorcilloSanz/WindowModule/blob/main/img/WindowModule.png)
### Create a window with an OpenGL context and handle its events for Linux and Windows

## Compile
Windows MinGW
```
gcc main.c WindowModule.c -static-libgcc -std=c11 -mwindows -lopengl32 -lglu32 -g3 -o triangle
```
Windows Visual Studio
```
Linker > Input > Additional Dependencies
	opengl32.lib;glu32.lib;
```
Linux GNU GCC:
```
gcc main.c WindowModule.c -lX11 -lGL -lGLU -o triangle
```

## Example
```cpp
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
```
### Output
![alt text](https://github.com/MorcilloSanz/WindowModule/blob/main/img/triangle.png)
