#ifndef EVENTS_H_
#define EVENTS_H_

enum event_type {
        EVENT_MOUSEMOVE,
        EVENT_BUTTONPRESS,
        EVENT_BUTTONRELEASE,
        EVENT_KEYPRESS,
        EVENT_KEYRELEASE,
};

enum button_type {
        BUTTON_1,
        BUTTON_2,
        BUTTON_3,
        BUTTON_4,
        BUTTON_5,
};

enum key_type {
        KEY_ESCAPE,
        KEY_SPACE,
        KEY_q,
};

struct mouse_event {
        float ratiox;
        float ratioy;
};

struct button_press_event {
        enum button_type btn;
};

struct button_release_event {
        enum button_type btn;
};

struct key_press_event {
        enum key_type key;
};

struct key_release_event {
        enum key_type key;
};

struct event {
        enum event_type evtp;
        union {
                struct mouse_event mouse_event;
                struct button_press_event button_press_event;
                struct button_release_event button_release_event;
                struct key_press_event key_press_event;
                struct key_release_event key_release_event;
        };
};

int  events_init(void);
void events_exit(void);
int  events_start_producing(void);
void events_stop_producing(void);
int  events_dequeue_if_avail(struct event *event);
int  user_wants_to_quit(void);

#endif
