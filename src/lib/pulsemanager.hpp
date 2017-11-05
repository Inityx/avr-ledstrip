#ifndef PULSEMANAGER_H
#define PULSEMANAGER_H

#include <stdint.h>
#include <stddef.h>

#include "portlib.hpp"

namespace PulseManager {
    using PortLib::DigitalPort;

    template<uint8_t COUNT>
    struct MultiPWM {
    private:
        uint8_t levels[COUNT];
        uint8_t pins[COUNT];
        DigitalPort port;
        uint8_t counter;
        uint8_t selection;
        
    public:
        constexpr MultiPWM(
            const uint8_t *pins_source,
            const uint8_t *levels_source,
            const DigitalPort &port
        ) : counter(0), port(port), selection(0) {
            // This loop is because you can't pass a C array
            // as an argument in C++ and std::array does not
            // exist on AVR. It's still constexpr though.
            for(uint8_t i{0}; i<COUNT; i++) {
                levels[i] = levels_source[i];
                pins[i] = pins_source[i];
            }
        }

        void set_level(uint8_t index, uint8_t value) {
            levels[index] = value;
        }
        
        void step() {
            if(counter == 0)
                // Set all pins high to start
                for(uint8_t i{0}; i<COUNT; i++)
                    port.set_high(pins[i]);
            else
                // Set pins low if count passed level
                for(uint8_t i{0}; i<COUNT; i++)
                    if(counter > levels[i])
                        port.set_low(pins[i]);
            
            counter++;
        }

        void select_next() {
            if (selection < (COUNT-1))
                selection++;
            else
                selection = 0;
        }

        void adjust_up()   { levels[selection]++; }
        void adjust_down() { levels[selection]--; }
    };
}
#endif
