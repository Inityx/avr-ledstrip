#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

#include "chips/attiny84.hpp"

#include "portlib/portlib.hpp"
#include "portlib/digitalpin.hpp"

#include "emulated/multipwm.hpp"

#include "peripheral/rotaryencoder.hpp"

#include "utility/array.hpp"

using namespace AVRSupport;

using PortLib::DigitalPin;
using PortLib::PinIndex;
using Peripheral::RotaryEncoder;
using Emulated::MultiPWM;
using Utility::Array;

PinIndex const CHANNELS{3};

Array<PinIndex, CHANNELS> const PWM_PINS = { 0, 1, 2 };
Array<uint8_t, CHANNELS> const INIT_LEVELS = { 0x00, 0x00, 0x00 };

DigitalPin rotary_button{port_b, 0};
DigitalPin ext_button{port_a, 3};

DigitalPin rotary_a{port_b, 2};
DigitalPin rotary_b{port_b, 1};

RotaryEncoder rotary_encoder;

MultiPWM<CHANNELS, 16> pwm{
    PWM_PINS,
    INIT_LEVELS,
    port_a
};

ISR(TIM0_OVF_vect) { pwm.step(); }

static inline void handle_buttons(bool rotary, bool ext_button) {
    if (rotary) {
        pwm.active = false;
        
        pwm.select_next();
        pwm.isolate_selected();
        _delay_ms(500);

        pwm.active = true;
    }

    if (ext_button) { /* Select next preset */ }
}

static inline void handle_rotary(bool right, bool left) {
    if (right || left) {
        if (right)     pwm.adjust_up();
        else if (left) pwm.adjust_down();
        rotary_encoder.clear();
    }
}

int main() {
    for(PinIndex i : PWM_PINS) port_a.set_out(i);

    timer_0.set_mode(Timer0::Mode::normal);
    timer_0.set_prescale(Timer0::Prescale::d1);
    timer_0.enable_interrupt(Timer0::Trigger::overflow);

    rotary_b.set_in();
    rotary_a.set_in();
    ext_button.set_in();
    rotary_button.set_in();
    rotary_button.set_high();
    
    sei();

    while(true) {
        // Polling
        rotary_encoder.update(!rotary_a.get(), !rotary_b.get());
        bool rotary_clicked = !rotary_button.get();
        bool ext_button_pressed = ext_button.get();

        // Handling
        handle_buttons(
            rotary_clicked,
            ext_button_pressed
        );
        handle_rotary(
            rotary_encoder.turned_right(),
            rotary_encoder.turned_left()
        );

        // All debounce
        _delay_us(20);
    }
}
