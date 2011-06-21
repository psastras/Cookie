#include <stdio.h>
#include <stdlib.h>
#include <X11/X.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/Xutil.h>
#include <sys/time.h>
#include "glcommon.h"
#include "glengine.h"
#include <GL/glx.h>
#include <sys/resource.h>
#include <string.h>
#include <sstream>
#include "common.h"
#include "keyboardcontroller.h"

bool checkGLXExtension(const char* extName, Display *dpy, int screen) {
  /*
    Search for extName in the extensions string.  Use of strstr()
    is not sufficient because extension names can be prefixes of
    other extension names.  Could use strtok() but the constant
    string returned by glGetString can be in read-only memory.
  */

  char* list = (char*) glXQueryExtensionsString(dpy, screen);
  char* end;
  int extNameLen;

  extNameLen = strlen(extName);
  end = list + strlen(list);

  while (list < end)  {
    int n = strcspn(list, " ");
    if ((extNameLen == n) && (strncmp(extName, list, n) == 0))
      return true;
    list += (n + 1);
  };
  return false;
};

int main(int argc, char *argv[]) {
    int which = PRIO_PROCESS;
    id_t pid;
    int priority = -20;
    int ret;

    pid = getpid();
    ret = setpriority(which, pid, priority);

     Display *dpy = XOpenDisplay(NULL);
     KeyboardController *keycontroller = new KeyboardController();
     if(dpy == NULL) {
	    cerr << "Cannot connect to X server.  (Are you running a window system?)" << endl;
	    exit(1);
     }
     Window root = DefaultRootWindow(dpy);
     GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
     XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
     if(vi == NULL) {
	    cerr << "No appropriate visual information found (OpenGL context intialization failed).  Exiting." << endl;
	    exit(1);
     }

     Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
     XSetWindowAttributes swa;
     swa.colormap = cmap;
     swa.event_mask = ExposureMask | KeyPressMask;

     Window win = XCreateWindow(dpy, root, 0, 0, 1366, 768, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
     XMapWindow(dpy, win);
     XStoreName(dpy, win, "OpenGL Water Demo [2011 - psastras]");
     GLXContext glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
     glXMakeCurrent(dpy, win, glc);
     XWindowAttributes gwa;
     XEvent xev;
     timespec ts;

     WindowProperties properties = {1366, 768};
     GLEngine eng(properties);

     // load fonts
     XFontStruct *font = XLoadQueryFont(dpy, "fixed");
     GLuint listbase = glGenLists(96);
     glXUseXFont(font->fid, ' ', 96, listbase);

     // test for vsync
     void (*swapInterval)(int) = 0;
     if (checkGLXExtension("GLX_MESA_swap_control", dpy, 0))
       swapInterval = (void (*)(int)) glXGetProcAddress((const GLubyte*) "glXSwapIntervalMESA");
     else if (checkGLXExtension("GLX_SGI_swap_control", dpy, 0))
       swapInterval = (void (*)(int)) glXGetProcAddress((const GLubyte*) "glXSwapIntervalSGI");
     else
       cerr << "No vsync detected, disabling." << endl;
     if(swapInterval) swapInterval(1);
     float dt = 0.f;

     XSelectInput(dpy, win, KeyPressMask | KeyReleaseMask); //subscribe to keypress and release

     //hide cursor
     Cursor invisibleCursor;
     Pixmap bitmapNoData;
     XColor black;
     static char noData[] = { 0,0,0,0,0,0,0,0 };
     black.red = black.green = black.blue = 0;

     bitmapNoData = XCreateBitmapFromData(dpy, win, noData, 8, 8);
     invisibleCursor = XCreatePixmapCursor(dpy, bitmapNoData, bitmapNoData,
					  &black, &black, 0, 0);
     XDefineCursor(dpy,win, invisibleCursor);
     XFreeCursor(dpy, invisibleCursor);
    float totaltime = 0.f;
    // main event loop
     while(1) {

	    while(XPending(dpy)) { //if we have a pending xevent
		XNextEvent(dpy, &xev);
		if(xev.type == Expose) { // gl context resized
			XGetWindowAttributes(dpy, win, &gwa);
			eng.resize(gwa.width, gwa.height);
			XWarpPointer(dpy, win, win, 0, 0, gwa.width, gwa.height, gwa.width / 2, gwa.height / 2);
		} else if(xev.type == KeyPress) {
		    //cout << "press: " << xev.xkey.keycode << endl;
		    keycontroller->keyPressEvent(xev.xkey.keycode);
		} else if(xev.type == KeyRelease) {
		//    cout << "release: " << xev.xkey.keycode << endl;
		    keycontroller->keyReleaseEvent(xev.xkey.keycode);
		}
	    }


	    Window winfocused;
	    int    revert;
	    XGetInputFocus(dpy, &winfocused, &revert);
	    if(winfocused == win) { //if window is focused keep the mouse steady and respond to mouse events
		XQueryPointer(dpy, win, &xev.xbutton.root, &xev.xbutton.window, &xev.xbutton.x_root, &xev.xbutton.y_root, &xev.xbutton.x, &xev.xbutton.y, &xev.xbutton.state);
		XWarpPointer(dpy, win, win, 0, 0, gwa.width, gwa.height, gwa.width / 2, gwa.height / 2);
		float dx = (xev.xbutton.x - (gwa.width / 2.0)) / (float)gwa.width;
		float dy = (xev.xbutton.y - (gwa.height / 2.0)) / (float)gwa.height;
		eng.mouseMove(dx, dy, dt);
	    }

	    // handle keyboard - @todo: remove hard coded values
	    if(keycontroller->isKeyDown(9)) { //esc
		break;
	    }
	    //draw
	    clock_gettime(CLOCK_REALTIME, &ts);
	    long time = ts.tv_nsec;

	    eng.draw(totaltime, dt, keycontroller);

	    clock_gettime(CLOCK_REALTIME, &ts);
	    std::stringstream ss;
	    dt = ((ts.tv_nsec - time) * 1.0e-9);
	    if(dt < 0) dt = 0.01; //@todo this is cause were overflowing max long i think?
	    ss << (int)(1.0 / dt) << " fps";
	    totaltime += dt;
	    const char *s = ss.str().c_str();
	    glPushAttrib(GL_LIST_BIT);
	    glListBase(listbase - 32);
	    glRasterPos2f(10.0, 20.0);
	    glCallLists(strlen(s), GL_BYTE, s);
	    glPopAttrib();
	    glListBase(0);

	    glXSwapBuffers(dpy, win);
	    glFinish();

	    keycontroller->swapBuffers();

    }

     delete keycontroller;
     // Restore the X left facing cursor
     Cursor cursor;
     cursor=XCreateFontCursor(dpy,XC_left_ptr);
     XDefineCursor(dpy, win, cursor);
     XFreeCursor(dpy, cursor);
     XUndefineCursor(dpy, win);
     glXMakeCurrent(dpy, None, NULL);
     glXDestroyContext(dpy, glc);
     XDestroyWindow(dpy, win);
     XCloseDisplay(dpy);
     return 0;
}
