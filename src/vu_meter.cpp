#include <math.h>
#include "flgui.h"

#include "vu_meter.h"


vu_led_t vu_led[9];

int left_cur_peak_led = 0;
int right_cur_peak_led = 0;

void vu_off_timer(void*);
void vu_left_peak_timer(void*);
void vu_right_peak_timer(void*);

void vu_init(void)
{
    int i;

    for(i = 0; i < 9; i++)
    {
        vu_led[0].left.state = 0;
        vu_led[0].right.state = 0;
        vu_led[0].left.is_peak = 0;
        vu_led[0].right.is_peak = 0;
    }

    vu_led[0].left.widget = fl_g->left_1_light; 
    vu_led[0].right.widget = fl_g->right_1_light; 
    vu_led[0].thld = TRESHOLD_1;

    vu_led[1].left.widget = fl_g->left_2_light; 
    vu_led[1].right.widget = fl_g->right_2_light; 
    vu_led[1].thld = TRESHOLD_2;

    vu_led[2].left.widget = fl_g->left_3_light; 
    vu_led[2].right.widget = fl_g->right_3_light; 
    vu_led[2].thld = TRESHOLD_3;

    vu_led[3].left.widget = fl_g->left_4_light; 
    vu_led[3].right.widget = fl_g->right_4_light; 
    vu_led[3].thld = TRESHOLD_4;

    vu_led[4].left.widget = fl_g->left_5_light; 
    vu_led[4].right.widget = fl_g->right_5_light; 
    vu_led[4].thld = TRESHOLD_5;

    vu_led[5].left.widget = fl_g->left_6_light; 
    vu_led[5].right.widget = fl_g->right_6_light; 
    vu_led[5].thld = TRESHOLD_6;

    vu_led[6].left.widget = fl_g->left_7_light; 
    vu_led[6].right.widget = fl_g->right_7_light; 
    vu_led[6].thld = TRESHOLD_7;

    vu_led[7].right.widget = fl_g->right_8_light; 
    vu_led[7].left.widget = fl_g->left_8_light; 
    vu_led[7].thld = TRESHOLD_8;

    vu_led[8].left.widget = fl_g->left_9_light; 
    vu_led[8].right.widget = fl_g->right_9_light; 
    vu_led[8].thld = TRESHOLD_9;


    Fl::add_timeout(0.1, &vu_off_timer);
}

void vu_off_timer(void*)
{
    int i;

    // Deactivate the LED rows step by step
    // Everytime this timer is executed only the last active non-peak LED is deactivated
    for(i = 8; i >= 0; i--)
    {
        if((vu_led[i].left.state == ON) && (vu_led[i].left.is_peak == 0))
        {
            vu_led[i].left.widget->hide();
            vu_led[i].left.state = OFF;
            break;
        }
    }
    for(i = 8; i >= 0; i--)
    {
        if((vu_led[i].right.state == ON) && (vu_led[i].right.is_peak == 0))
        {
            vu_led[i].right.widget->hide();
            vu_led[i].right.state = OFF;
            break;
        }
    }

    Fl::repeat_timeout(0.1, &vu_off_timer);
}

void vu_meter(short left, short right)
{

    int i;
    float left_db;
    float right_db;


    // Convert the 16bit integer values into dB values
    if (left > 0)
        left_db = -(20 * log10(32768/(float)left))+VU_OFFSET;
    else
        left_db = -1000; //-inf

    if (right > 0)
        right_db = -(20 * log10(32768/(float)right))+VU_OFFSET;
    else
        right_db = -1000;

    
    // Activate all LEDs whose threshold is exceeded by the input sample
    for(i = 0; i < 9; i++) 
    {
        if(left_db > vu_led[i].thld)
        {
            vu_led[i].left.widget->show();
            vu_led[i].left.state = ON;

            // keep the peak value up to date
            if(i > left_cur_peak_led)
            {
                // clear old peak value
                vu_led[left_cur_peak_led].left.is_peak = 0;
                
                // set new peak
                vu_led[i].left.is_peak = 1;
                left_cur_peak_led = i;
              
                // retrigger timeout for new peak led
                Fl::remove_timeout(&vu_left_peak_timer);
                Fl::add_timeout(1/*second*/, &vu_left_peak_timer);

            }
        }
        if(right_db > vu_led[i].thld)
        {
            vu_led[i].right.widget->show();
            vu_led[i].right.state = ON;
            if(i > right_cur_peak_led)
            {
                vu_led[right_cur_peak_led].right.is_peak = 0;
                
                vu_led[i].right.is_peak = 1;
                right_cur_peak_led = i;
                
                Fl::remove_timeout(&vu_right_peak_timer);
                Fl::add_timeout(1, &vu_right_peak_timer);
            }
        }
    }
}

void vu_left_peak_timer(void*)
{
    // Deactivate the peak led 
    vu_led[left_cur_peak_led].left.is_peak = 0;
    vu_led[left_cur_peak_led].left.state = OFF;
    vu_led[left_cur_peak_led].left.widget->hide();
    left_cur_peak_led = 0;
}

void vu_right_peak_timer(void*)
{
    vu_led[right_cur_peak_led].right.is_peak = 0;
    vu_led[right_cur_peak_led].right.state = OFF;
    vu_led[right_cur_peak_led].right.widget->hide();
    right_cur_peak_led = 0;
}
