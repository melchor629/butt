#ifndef FL_MY_VALUE_SLIDER
#define FL_MY_VALUE_SLIDER


#include <stdlib.h>
#include <string.h>
#include <FL/fl_draw.H>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Menu_Window.H>
#include <FL/Fl_Tooltip.H>
// FLOATING TIP WINDOW
class TipWin : public Fl_Menu_Window {
    char tip[40];
public:
    TipWin():Fl_Menu_Window(1,1) {      // will autosize
        strcpy(tip, "X.XX");
        set_override();
        end();
    }
    void draw(void) {
        draw_box(FL_BORDER_BOX, 0, 0, w(), h(), Fl_Color(175));
        fl_color(FL_BLACK);
        fl_font(labelfont(), labelsize());
        fl_draw(tip, 3, 3, w()-6, h()-6, Fl_Align(FL_ALIGN_LEFT|FL_ALIGN_WRAP));
    }
    void value(float f) {
        sprintf(tip, "%.2fdB", f);
        // Recalc size of window
        fl_font(labelfont(), labelsize());
        int W = w(), H = h();
        fl_measure(tip, W, H, 0);
        W += 8;
        size(W, H);
        redraw();
    }
};
// VALUE SLIDER WITH FLOATING TIP WINDOW
class Fl_My_Value_Slider : public Fl_Slider {
    TipWin *tipwin;
public:
    void value_cb2(void) {
        tipwin->value(value());
        tipwin->position(Fl::event_x_root()+20, Fl::event_y_root());
    }
    Fl_My_Value_Slider(int x,int y,int w,int h,const char*l=0):Fl_Slider(x,y,w,h,l) {
        type(FL_HOR_SLIDER);
        Fl_Group *save = Fl_Group::current();   // save current widget..
        tipwin = new TipWin();                  // ..because this trashes it
        tipwin->hide();
        Fl_Group::current(save);                // ..then back to previous.
    }
    int handle(int e) {
        switch(e) {
            case FL_PUSH:
                if(Fl::event_clicks()) //move slider to zero when double clicking
                {
                    this->value(0.0);
                    this->parent()->redraw();
                    this->do_callback();
                    return 1;
                }
                tipwin->position(Fl::event_x_root(), Fl::event_y_root()+20);
                tipwin->value(value());
                tipwin->show();
                break;
            case FL_HIDE:       // valuator goes away
            case FL_RELEASE:    // release mouse
            case FL_LEAVE:      // leave focus
                // Make sure tipwin closes when app closes
                tipwin->hide();
                break;
        }
        return(Fl_Slider::handle(e));
    }
};

#endif
