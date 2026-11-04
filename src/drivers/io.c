#include "drivers/io.h"
#include "common/defines.h"

#include <msp430.h>
#include <stdint.h>

#define IO_PORT_CNT (2u)
#define IO_PIN_CNT_PER_PORT (8u)

/* With "-fshort-enums", enum values are one byte; encoding:
 * [ zeros (3) | port (2) | pin (3) ] */
/*  CBadweh's Note
    Bitsise Operation (Code Trick) at timestamp 15:40
    pin = enum & 0x7
    pin_bit = 0x1 << pin
    port = (enum & (0x3<<3))
*/
#define IO_PORT_OFFSET (3u)
#define IO_PORT_MASK (0x3u << IO_PORT_OFFSET)
#define IO_PIN_MASK (0x7u)

static inline uint8_t io_port(io_e io)
{
    return (io & IO_PORT_MASK) >> IO_PORT_OFFSET;
}

static inline uint8_t io_pin_idx(io_e io)
{
    return io & IO_PIN_MASK;
}

static inline uint8_t io_pin_bit(io_e io)
{
    return 1 << io_pin_idx(io);
}

static volatile uint8_t *const port_dir_regs[IO_PORT_CNT] = { &P1DIR, &P2DIR };
static volatile uint8_t *const port_ren_regs[IO_PORT_CNT] = { &P1REN, &P2REN };
static volatile uint8_t *const port_out_regs[IO_PORT_CNT] = { &P1OUT, &P2OUT };
static volatile uint8_t *const port_in_regs[IO_PORT_CNT] = { &P1IN, &P2IN };
static volatile uint8_t *const port_sel1_regs[IO_PORT_CNT] = { &P1SEL, &P2SEL };
static volatile uint8_t *const port_sel2_regs[IO_PORT_CNT] = { &P1SEL2, &P2SEL2 };

/* Unused pins should be
 * "Switched to port function, output direction or input with pullup/pulldown enabled"
 * according to the datasheet (2.5). Importantly, they should not be left as floating
 * inputs because that leads to unpredictable (noise) current consumption. I choose to
 * configure them as output (instead of input) to lower the risk of short-circuit, and
 * pull them down. */
#define UNUSED_CONFIG                                                                              \
    {                                                                                              \
        IO_SELECT_GPIO, IO_RESISTOR_ENABLED, IO_DIR_OUTPUT, IO_OUT_LOW                             \
    }

// This array holds the initial configuration for all IO pins.
static const struct io_config io_initial_configs[IO_PORT_CNT * IO_PIN_CNT_PER_PORT] = {
    // Output
    [IO_TEST_LED] = { IO_SELECT_GPIO, IO_RESISTOR_DISABLED, IO_DIR_OUTPUT, IO_OUT_LOW },

    /* UART RX/TX
     * Resistor: Not needed (pulled by transmitter/receiver)
     * Direction: Not applicable
     * Output: Not applicable */
    [IO_UART_RXD] = { IO_SELECT_ALT3, IO_RESISTOR_DISABLED, IO_DIR_OUTPUT, IO_OUT_LOW },
    [IO_UART_TXD] = { IO_SELECT_ALT3, IO_RESISTOR_DISABLED, IO_DIR_OUTPUT, IO_OUT_LOW },

    // Unused pins
    [IO_UNUSED_1] = UNUSED_CONFIG,
    [IO_UNUSED_2] = UNUSED_CONFIG,
    [IO_UNUSED_3] = UNUSED_CONFIG,
    [IO_UNUSED_4] = UNUSED_CONFIG,
    [IO_UNUSED_5] = UNUSED_CONFIG,
    [IO_UNUSED_6] = UNUSED_CONFIG,
    [IO_UNUSED_7] = UNUSED_CONFIG,
    [IO_UNUSED_8] = UNUSED_CONFIG,
    [IO_UNUSED_9] = UNUSED_CONFIG,
    [IO_UNUSED_10] = UNUSED_CONFIG,
    [IO_UNUSED_11] = UNUSED_CONFIG,
    [IO_UNUSED_12] = UNUSED_CONFIG,
    [IO_UNUSED_13] = UNUSED_CONFIG,
};

void io_init(void)
{
    // TODO: Loop initialize all pins
    for (io_e io = (io_e)IO_10; io < ARRAY_SIZE(io_initial_configs); io++) {
        io_configure(io, &io_initial_configs[io]);
    }
}

void io_configure(io_e io, const struct io_config *config)
{
    io_set_select(io, config->select);
    io_set_direction(io, config->dir);
    io_set_out(io, config->out);
    io_set_resistor(io, config->resistor);
}

void io_set_select(io_e io, io_select_e select)
{
    const uint8_t port = io_port(io);
    const uint8_t pin = io_pin_bit(io);
    switch (select) {
    case IO_SELECT_GPIO:
        *port_sel1_regs[port] &= ~pin;
        *port_sel2_regs[port] &= ~pin;
        break;
    case IO_SELECT_ALT1:
        *port_sel1_regs[port] |= pin;
        *port_sel2_regs[port] &= ~pin;
        break;
    case IO_SELECT_ALT2:
        *port_sel1_regs[port] &= ~pin;
        *port_sel2_regs[port] |= pin;
        break;
    case IO_SELECT_ALT3:
        *port_sel1_regs[port] |= pin;
        *port_sel2_regs[port] |= pin;
        break;
    }
}

void io_set_direction(io_e io, io_dir_e direction)
{
    const uint8_t port = io_port(io);
    const uint8_t pin = io_pin_bit(io);
    switch (direction) {
    case IO_DIR_OUTPUT:
        *port_dir_regs[port] |= pin;
        break;
    case IO_DIR_INPUT:
        *port_dir_regs[port] &= ~pin;
        break;
    }
}

void io_set_resistor(io_e io, io_resistor_e resistor)
{
    const uint8_t port = io_port(io);
    const uint8_t pin = io_pin_bit(io);
    switch (resistor) {
    case IO_RESISTOR_ENABLED:
        *port_ren_regs[port] |= pin;
        break;
    case IO_RESISTOR_DISABLED:
        *port_ren_regs[port] &= ~pin;
        break;
    }
}

void io_set_out(io_e io, io_out_e out)
{
    const uint8_t port = io_port(io);
    const uint8_t pin = io_pin_bit(io);
    switch (out) {
    case IO_OUT_HIGH:
        *port_out_regs[port] |= pin;
        break;
    case IO_OUT_LOW:
        *port_out_regs[port] &= ~pin;
        break;
    }
}

io_in_e io_get_input(io_e io)
{
    return (*port_in_regs[io_port(io)] & io_pin_bit(io)) ? IO_IN_HIGH : IO_IN_LOW;
    return 0;
}