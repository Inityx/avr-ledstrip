#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

#include "chips/attiny84.hpp"
#include "portlib/portlib.hpp"
#include "portlib/digitalpin.hpp"
#include "emulated/multipwm.hpp"
#include "peripheral/rotaryencoder.hpp"

using namespace AvrSupport;

using PortLib::DigitalPin;
using PortLib::PinIndex;
using Peripheral::RotaryEncoder;
using Emulated::MultiPWM;

MultiPWM<3, 16> pwm{
    {0, 1, 2},
    {0, 0, 0},
    port_a
};

ISR(TIM0_OVF_vect) { pwm.step(); }

static inline void handle_buttons(bool knob_button, bool aux_button) {
    if (knob_button) {
        pwm.pause();
        
        pwm.select_next();
        pwm.isolate_selected();
        _delay_ms(500);

        pwm.resume();
    }

    if (aux_button) {
        // TODO: Select next preset
    }
}

static inline void handle_knob_rotate(RotaryEncoder & knob) {
    if (knob.turned_right()) { pwm.adjust_up();   knob.clear(); return; }
    if (knob.turned_left ()) { pwm.adjust_down(); knob.clear(); return; }
}

int main() {
    RotaryEncoder knob;

    DigitalPin knob_pin_a {port_b, 2};
    DigitalPin knob_pin_b {port_b, 1};
    DigitalPin knob_button{port_b, 0};
    DigitalPin  aux_button{port_a, 3};

    knob_pin_b .set_in();
    knob_pin_a .set_in();
    knob_button.set_in();
    knob_button.set_high();
     aux_button.set_in();
    
    pwm.set_pins_out();

    timer_0.set_mode        (Timer0::Mode::normal);
    timer_0.set_prescale    (Timer0::Prescale::d1);
    timer_0.enable_interrupt(Timer0::Trigger::overflow);

    sei();

    while(true) {
        bool  aux_button_clicked =   aux_button.get();
        bool knob_button_clicked = !knob_button.get();

        knob.update(!knob_pin_a.get(), !knob_pin_b.get());

        handle_buttons(knob_button_clicked, aux_button_clicked);
        handle_knob_rotate(knob);

        // Debounce
        _delay_us(20);
    }
}
