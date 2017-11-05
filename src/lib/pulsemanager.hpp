#ifndef PULSEMANAGER_H
#define PULSEMANAGER_H

#include <stdint.h>
#include <stddef.h>

#include "portlib.hpp"

namespace PulseManager {
    using PortLib::DigitalPort;
    using PortLib::PinIndex;

    template<PinIndex COUNT, uint8_t STEP>
    struct MultiPWM {
    private:
        uint8_t levels[COUNT];
        PinIndex pins[COUNT];
        DigitalPort &port;
        uint8_t counter;
        
    public:
        bool active;
        PinIndex selection;
        
        constexpr MultiPWM(
            PinIndex const *pins_source,
            uint8_t const *levels_source,
            DigitalPort &port
        ) : counter(0), port(port), active(true), selection(0) {
            // This loop is because you can't pass a C array
            // as an argument in C++ and std::array does not
            // exist on AVR. It's still constexpr though.
            for(PinIndex i{0}; i<COUNT; i++) {
                levels[i] = levels_source[i];
                pins[i] = pins_source[i];
            }
        }

        inline void set_level(PinIndex index, uint8_t value) {
            levels[index] = value;
        }
        
        inline void isolate_selected() {
            for(uint8_t i{0}; i<COUNT; i++)
                if (i != selection)
                    port.set_low(pins[i]);

            port.set_high(pins[selection]);
        }

        inline void step() {
            if (!active) return;

            // Set all pins high to start
            if(counter == 0)
                for(PinIndex i{0}; i<COUNT; i++)
                    if (levels[i] != 0)
                        port.set_high(pins[i]);

            // Set pins low if count passed level
            for(PinIndex i{0}; i<COUNT; i++)
                if(counter > levels[i])
                    port.set_low(pins[i]);
            
            counter++;
        }

        inline void select_next() {
            if (selection >= COUNT-1) {
                selection = 0;
                return;
            }

            selection++;
        }

        inline void adjust_up()   { levels[selection] += STEP; }
        inline void adjust_down() { levels[selection] -= STEP; }
    };
}
#endif
