#ifndef EVENTS_H_
#define EVENTS_H_

enum event_type {
        EVENT_MOUSEMOVE,
};

struct mouse_event {
        float ratiox;
        float ratioy;
};

struct event {
        enum event_type evtp;
        union {
                struct mouse_event mouse_event;
        };
};

int  events_init(void);
void events_exit(void);
int  events_start_producing(void);
void events_stop_producing(void);
int  events_dequeue_if_avail(struct event *event);

#endif
