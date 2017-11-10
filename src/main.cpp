#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

#include "lib/portlib.hpp"
#include "lib/pulsemanager.hpp"

using PortLib::DigitalPort;
using PortLib::DigitalPin;
using PortLib::RotaryEncoder;
using PortLib::PinIndex;
using PortLib::Timer0;
using PulseManager::MultiPWM;

PinIndex const CHANNELS{3};

PinIndex const PWM_PINS[CHANNELS] = { 0, 1, 2 };
uint8_t const INIT_LEVELS[CHANNELS] = { 0x00, 0x00, 0x00 };

DigitalPort port_a{ PINA, PORTA, DDRA };
DigitalPort port_b{ PINB, PORTB, DDRB };

DigitalPin rotary_button{port_b, 0};
DigitalPin ext_button{port_a, 3};

DigitalPin rotary_a{port_b, 2};
DigitalPin rotary_b{port_b, 1};

RotaryEncoder rotary_encoder{
    [](){ return rotary_a.get(); },
    [](){ return rotary_b.get(); }
};

MultiPWM<CHANNELS, 32> pwm{
    PWM_PINS,
    INIT_LEVELS,
    port_a
};

Timer0 pwm_timer{
    TCNT0,
    OCR0A, OCR0B,
    TCCR0A, TCCR0B,
    TIMSK0,
    TIFR0
};

ISR(TIM0_OVF_vect) { pwm.step(); }

static inline void event_handler(
    bool rotary_clicked,
    bool ext_button_pressed,
    bool rotary_transient
) {
    if (rotary_clicked || ext_button_pressed) {
        // If caused by button press
        pwm.active = false;

        if (rotary_clicked) pwm.select_next();
        if (ext_button_pressed) { /* Select next preset */ }
        
        pwm.isolate_selected();
        _delay_ms(500);
        
        pwm.active = true;
    }

    if (rotary_transient) {
        // If caused by rotary encoder
        switch (rotary_encoder.get_active()) {
        case RotaryEncoder::Active::A:
            pwm.adjust_up();
            break;

        case RotaryEncoder::Active::B:
            pwm.adjust_down();
            break;

        default: break;
        }
        
        // debounce
        _delay_ms(20);
    }
}

int main() {
    for(PinIndex i{0}; i<sizeof(PWM_PINS); i++)
        port_a.set_out(PWM_PINS[i]);

    pwm_timer.set_mode(Timer0::Mode::clear_on_compare);
    pwm_timer.set_prescale(Timer0::Prescale::d1);
    pwm_timer.enable_interrupt(Timer0::Trigger::overflow);

    rotary_b.set_in();
    rotary_a.set_in();
    ext_button.set_in();
    
    sei();

    while(true) {
        // Polling
        rotary_encoder.update();

        bool rotary_transient = rotary_encoder.rising_edge();
        bool rotary_clicked = !rotary_button.get(); // Default high
        bool ext_button_pressed = ext_button.get(); // Default low

        // Event detection
        if (
            rotary_clicked ||
            ext_button_pressed ||
            rotary_transient
        ) {
            event_handler(
                rotary_clicked,
                ext_button_pressed,
                rotary_transient
            );
        }
    }
}
