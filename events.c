#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include "events.h"
#include "lockfree_fifo.h"

/* It seems cancelling the writer thread is not possible when it is inside Xlib
 * (e.g. XNextEvent()). So we use shared variables to communicate exit request.
 * The problem currently is that we have to wait for XNextEvent() to return
 * before the thread exits. */

#define EVENT_BUFFER_SIZE (128)

static pthread_t creator_thread_var;
static Display *dpy;
static Window win;
static Atom delete_window_atom;
static int mousex = 0;
static int mousey = 0;
static int width = 400;
static int height = 400;
static struct lockfree_fifo *fifo;
/* creator thread <-> extern calls communication variables */
static volatile int quit_key_pressed;
static volatile int delete_window_received;
static volatile int creator_thread_must_exit;

static int init_display(void)
{
        dpy = XOpenDisplay(NULL);
        if (!dpy) {
                fprintf(stderr, "Failed to open display\n");
                return -1;
        }
        win = XCreateSimpleWindow(
                dpy, DefaultRootWindow(dpy), 0, 0, width, height, 0, 0, 0);
        XSelectInput(dpy, win, PointerMotionMask | StructureNotifyMask
                                | KeyPressMask | KeyReleaseMask);
        XMapWindow(dpy, win);

        delete_window_atom = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(dpy, win, &delete_window_atom, 1);

        return 0;
}

static void exit_display(void)
{
        XCloseDisplay(dpy);
}

static void enqueue_event(struct event *ev)
{
        int r = lockfree_fifo_enqueue(fifo, ev);
        if (r == -1) {
                fprintf(stderr, "WARNING: events buffer overrun\n");
        }
}

static void handle_configurenotify(XConfigureEvent *ev)
{
        width = ev->width;
        height = ev->height;
}

static void handle_motionnotify(XMotionEvent *xev)
{
        struct event ev;

        mousex = xev->x;
        mousey = xev->y;

        ev.evtp = EVENT_MOUSEMOVE;
        ev.mouse_event.ratiox = (float) xev->x / width;
        ev.mouse_event.ratioy = (float) xev->y / height;

        enqueue_event(&ev);
}

static int x11button_to_buttontype(unsigned b, enum button_type *out)
{
        switch (b) {
        case Button1Mask:
                *out = BUTTON_1;
                return 0;
        case Button2Mask:
                *out = BUTTON_2;
                return 0;
        case Button3Mask:
                *out = BUTTON_3;
                return 0;
        case Button4Mask:
                *out = BUTTON_4;
                return 0;
        case Button5Mask:
                *out = BUTTON_5;
                return 0;
        default:
                return -1;
        }
}

static int x11key_to_keytype(XKeyEvent *ev, enum key_type *out)
{
        switch (XLookupKeysym(ev, 0)) {
        case XK_Escape:
                *out = KEY_ESCAPE;
                return 0;
        case XK_space:
                *out = KEY_SPACE;
                return 0;
        case XK_q:
                *out = KEY_q;
                return 0;
        default:
                return -1;
        }
}

static void handle_buttonpress(XButtonEvent *xev)
{
        struct event ev;
        ev.evtp = EVENT_BUTTONPRESS;
        if (x11button_to_buttontype(xev->button, &ev.button_press_event.btn)
            == -1)
                return;
        enqueue_event(&ev);
}

static void handle_buttonrelease(XButtonEvent *xev)
{
}

static void handle_keypress(XKeyEvent *xev)
{
        enum key_type key;
        struct event ev;

        if (x11key_to_keytype(xev, &key) == -1)
                return;
        switch (key) {
        case KEY_ESCAPE:
        case KEY_q:
                quit_key_pressed = 1;
                break;
        case KEY_SPACE:
                ev.evtp = EVENT_KEYPRESS;
                ev.key_press_event.key = key;
                enqueue_event(&ev);
                break;
        default:
                break;
        }
}

static void handle_keyrelease(XKeyEvent *xev)
{
}

static void *creator_thread(void *dummy)
{
        (void) dummy;
        while (!creator_thread_must_exit) {
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
                        handle_keypress(&ev.xkey);
			break;
		case KeyRelease:
                        handle_keyrelease(&ev.xkey);
			break;
		case ButtonPress:
			handle_buttonpress(&ev.xbutton);
			break;
		case ButtonRelease:
                        handle_buttonrelease(&ev.xbutton);
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
                        if ((Atom)ev.xclient.data.l[0] == delete_window_atom)
                                delete_window_received = 1;
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
        int r;

        creator_thread_must_exit = 1;
        r = pthread_join(creator_thread_var, NULL);
        if (r != 0) {
                fprintf(stderr, "ERROR: Failed to join creator thread\n");
        }
}

int events_dequeue_if_avail(struct event *event)
{
        if (lockfree_fifo_dequeue(fifo, event) == -1)
                return -1;
        return 0;
}

int user_wants_to_quit(void)
{
        return quit_key_pressed || delete_window_received;
}

int events_init(void)
{
        if (init_display() == -1) {
                return -1;
        }

        if (lockfree_fifo_init(&fifo, sizeof (struct event), EVENT_BUFFER_SIZE)
            == -1) {
                fprintf(stderr, "Failed to init fifo\n");
                exit_display();
                return -1;
        }

        return 0;
}

void events_exit(void)
{
        lockfree_fifo_exit(fifo);
        exit_display();
}
