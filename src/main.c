#include <msp430.h>
#include "drivers/io.h"
#include "drivers/mcu_init.h"

static void test_setup(void)
{
    mcu_init();
}

// Configure all pins as output and then toggle them in a loop. Verify with a logic analyzer.
static void test_launchpad_io_pins_output(void)
{
    test_setup();
    const struct io_config output_config = { .select = IO_SELECT_GPIO,
                                            .resistor = IO_RESISTOR_DISABLED,
                                            .dir = IO_DIR_OUTPUT,
                                            .out = IO_OUT_LOW };

    // Configure all pins as output
    for (io_generic_e io = IO_10; io <= IO_27; io++)
    {
        io_configure(io, &output_config);
    }
    while (1) {
        for (io_generic_e io = IO_10; io <= IO_27; io++) {
            io_set_out(io, IO_OUT_HIGH);
            __delay_cycles(10000);
            io_set_out(io, IO_OUT_LOW);
        }
    }
}

int main(void)
{
    test_launchpad_io_pins_output();
    return 0;
}