#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "events.h"
#include "lockfree_fifo.h"

#define EVENT_BUFFER_SIZE (128)

static pthread_t creator_thread_var;
static Display *dpy;
static Window win;
static Atom wm_delete_window;
static int mousex = 0;
static int mousey = 0;
static int width = 400;
static int height = 400;
static struct lockfree_fifo *fifo;

static void handle_configurenotify(XConfigureEvent *ev)
{
        width = ev->width;
        height = ev->height;
}

static void handle_motionnotify(XMotionEvent *xev)
{
        int r;
        struct event ev;

        mousex = xev->x;
        mousey = xev->y;

        ev.evtp = EVENT_MOUSEMOVE;
        ev.mouse_event.ratiox = (float) xev->x / width;
        ev.mouse_event.ratioy = (float) xev->y / height;

        r = lockfree_fifo_enqueue(fifo, &ev);
        if (r == -1) {
                fprintf(stderr, "WARNING: events buffer overrun\n");
        }
        else {
                /*
                fprintf(stderr, "created mouse move event\n");
                */
        }
}

static void *creator_thread(void *dummy)
{
        (void) dummy;
        for (;;) {
                XEvent ev;
                XNextEvent(dpy, &ev);
                switch (ev.type) {
                case ConfigureNotify:
                        handle_configurenotify(&ev.xconfigure);
                        break;
                case MotionNotify:
                        handle_motionnotify(&ev.xmotion);
                        break;

		case KeyPress:
			fprintf(stderr, "KeyPress\n");
			break;
		case KeyRelease:
			fprintf(stderr, "KeyRelease\n");
			break;
		case ButtonPress:
			fprintf(stderr, "ButtonPress\n");
			break;
		case ButtonRelease:
			fprintf(stderr, "ButtonRelease\n");
			break;
		case EnterNotify:
			fprintf(stderr, "EnterNotify\n");
			break;
		case LeaveNotify:
			fprintf(stderr, "LeaveNotify\n");
			break;
		case FocusIn:
			fprintf(stderr, "FocusIn\n");
			break;
		case FocusOut:
			fprintf(stderr, "FocusOut\n");
			break;
		case KeymapNotify:
			fprintf(stderr, "KeymapNotify\n");
			break;
		case Expose:
			fprintf(stderr, "Expose\n");
			break;
		case GraphicsExpose:
			fprintf(stderr, "GraphicsExpose\n");
			break;
		case NoExpose:
			fprintf(stderr, "NoExpose\n");
			break;
		case CirculateRequest:
			fprintf(stderr, "CirculateRequest\n");
			break;
		case ConfigureRequest:
			fprintf(stderr, "ConfigureRequest\n");
			break;
		case MapRequest:
			fprintf(stderr, "MapRequest\n");
			break;
		case ResizeRequest:
			fprintf(stderr, "ResizeRequest\n");
			break;
		case CirculateNotify:
			fprintf(stderr, "CirculateNotify\n");
			break;
		case CreateNotify:
			fprintf(stderr, "CreateNotify\n");
			break;
		case DestroyNotify:
			fprintf(stderr, "DestroyNotify\n");
			break;
		case GravityNotify:
			fprintf(stderr, "GravityNotify\n");
			break;
		case MapNotify:
			/*fprintf(stderr, "MapNotify\n");
                        */
			break;
		case MappingNotify:
			fprintf(stderr, "MappingNotify\n");
			break;
		case ReparentNotify:
			/*fprintf(stderr, "ReparentNotify\n");
                         */
			break;
		case UnmapNotify:
			/*fprintf(stderr, "UnmapNotify\n");
                        */
			break;
		case VisibilityNotify:
			fprintf(stderr, "VisibilityNotify\n");
			break;
		case ColormapNotify:
			fprintf(stderr, "ColormapNotify\n");
			break;
		case ClientMessage:
			/*fprintf(stderr, "ClientMessage\n");
                         */
                        if ((Atom)ev.xclient.data.l[0] == wm_delete_window)
                                /* XXX */
                                exit(0);
			break;
		case PropertyNotify:
			fprintf(stderr, "PropertyNotify\n");
			break;
		case SelectionClear:
			fprintf(stderr, "SelectionClear\n");
			break;
		case SelectionNotify:
			fprintf(stderr, "SelectionNotify\n");
			break;
		case SelectionRequest:
			fprintf(stderr, "SelectionRequest\n");
			break;
                default:
                        fprintf(stderr, "unknown event\n");
                        break;
                }
        }
        return NULL;
}

int events_start_producing(void)
{
        int r;

        r = pthread_create(&creator_thread_var, NULL, creator_thread, NULL);
        if (r != 0) {
                fprintf(stderr, "failed to create events thread\n");
                return -1;
        }
        return 0;
}

void events_stop_producing(void)
{
        int r = pthread_cancel(creator_thread_var);
        if (r != 0) {
                fprintf(stderr, "ERROR: Failed to cancel thread\n");
                exit(1);
        }
}

int events_dequeue_if_avail(struct event *event)
{
        if (lockfree_fifo_dequeue(fifo, event) == -1)
                return -1;
        return 0;
}

int events_init(void)
{
        dpy = XOpenDisplay(NULL);
        if (!dpy) {
                fprintf(stderr, "Failed to open display\n");
                return -1;
        }
        win = XCreateSimpleWindow(
                dpy, DefaultRootWindow(dpy), 0, 0, width, height, 0, 0, 0);
        XSelectInput(dpy, win, PointerMotionMask | StructureNotifyMask);
        XMapWindow(dpy, win);

        wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(dpy, win, &wm_delete_window, 1);

        if (lockfree_fifo_init(&fifo, sizeof (struct event), EVENT_BUFFER_SIZE)
            == -1) {
                fprintf(stderr, "Failed to init fifo\n");
                XCloseDisplay(dpy);
                return -1;
        }

        return 0;
}

void events_exit(void)
{
        XCloseDisplay(dpy);
}
