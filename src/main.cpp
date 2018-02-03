#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

#include <portlib/digitalport.hpp>
#include <portlib/digitalpin.hpp>
#include <portlib/timer.hpp>
#include <utility/array.hpp>

#include <emulated/multipwm.hpp>
#include <peripheral/rotaryencoder.hpp>

using namespace AvrSupport;
using namespace PortLib;
using Utility::Array;
using Peripheral::RotaryEncoder;
using Timer0 = Timer<uint8_t>;

DigitalPort
    port_a { PINA, PORTA, DDRA},
    port_b { PINB, PORTB, DDRB};

Emulated::MultiPWM<3, 15> pwm{ // 255 is evenly divisible by 15
    {0, 1, 2}, // Pins
    {0, 0, 0}, // Levels
    port_a
};

Timer0 timer_0{
    TCNT0,
    OCR0A, OCR0B,
    TCCR0A, TCCR0B,
    TIMSK0,
    TIFR0
};

ISR(TIM0_OVF_vect) { pwm.step(); }

void handle_buttons(bool knob_button, bool aux_button) {
    if (knob_button) {
        pwm.pause();
        
        pwm.select_next();
        pwm.isolate_selected();
        _delay_ms(500);

        pwm.resume();
    }

    if (aux_button) {
        // TODO: Select next preset
        pwm.pause();
        pwm.isolate_selected();
        _delay_ms(1000);
        pwm.resume();
    }
}

void handle_knob_rotate(RotaryEncoder & knob) {
    if      (knob.turned_right()) pwm.adjust_up();
    else if (knob.turned_left ()) pwm.adjust_down();
    else return;

    knob.clear();
}

int main() {
    RotaryEncoder knob;

    DigitalPin
        knob_pin_a {port_b, 2},
        knob_pin_b {port_b, 1},
        knob_button{port_a, 4},
         aux_button{port_a, 3};

    Array<DigitalPin *, 4> inputs {
        &knob_pin_a,
        &knob_pin_b,
        &knob_button,
        &aux_button
    };

    for (auto input : inputs)
        (*input).set_in().set_high();
    
    pwm.set_pins_out();

    timer_0
        .set_mode        (Timer0::Mode::normal)
        .set_prescale    (Timer0::Prescale::d1)
        .enable_interrupt(Timer0::Trigger::overflow);

    sei();

    while(true) {
        handle_buttons(!knob_button.get(), !aux_button.get());
        knob.sample(!knob_pin_a.get(), !knob_pin_b.get());
        handle_knob_rotate(knob);

        // Debounce
        _delay_us(20);
    }
}
