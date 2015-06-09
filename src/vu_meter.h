#ifndef VU_METER
#define VU_METER

#include <FL/Fl_Widget.H>

#define VU_OFFSET 6

#define TRESHOLD_1  -50 //dB
#define TRESHOLD_2  -30
#define TRESHOLD_3  -20
#define TRESHOLD_4  -12 
#define TRESHOLD_5  -4
#define TRESHOLD_6  0
#define TRESHOLD_7  1
#define TRESHOLD_8  3
#define TRESHOLD_9  5.5

enum LED_state {
    OFF  = 0,
    ON   = 1
};

struct vu_led_t {

    float thld;

    struct {
        int state;
        Fl_Widget *widget;
        int is_peak;
    }left, right;
};

void vu_init(void);
void vu_meter(short left, short right);

#endif
