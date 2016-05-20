/*
    Copyright (c) 2016 Thomas Stilwell <stilwellt@openlabs.co>

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use,
    copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following
    conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.
*/

#include "Sensortag.h"

#include "board.h"
#include "periph_conf.h"
#include "periph/cpuid.h"
#include "periph/gpio.h"
#include "thread.h"
#include "xtimer.h"

#include <stdio.h>

#ifndef GIT_VERSION
#define GIT_VERSION "undefined"
#endif

Sensortag* sensortagS;

void debug_printf(const char *format, ...) {
//     tm time;
//     wtimer_get_localtime(&time);
//
//     va_list args;
//     va_start(args, format);
//     char new_format[256];
//     snprintf(new_format, sizeof(new_format), "%04i-%02i-%02i %02i:%02i:%02i %s",
//              time.tm_year, time.tm_mon, time.tm_mday, time.tm_hour,
//              time.tm_min, time.tm_sec, format);
// //     snprintf(new_format, 256, "%s", format);
//     vprintf(new_format, args);
//     va_end(args);
}

// #define DEBUG(...) debug_printf(__VA_ARGS__)
#define DEBUG(...) printf(__VA_ARGS__)

void flash_led(gpio_t led, uint flashes) {
    for (uint flash = 0; flash < flashes; flash++) {
        gpio_set(led);
        xtimer_usleep(200*1000);
        gpio_clear(led);
        xtimer_usleep(150*1000);
    }
}

Sensortag::Sensortag() :
main_pid(thread_getpid())
{
    sensortagS = this;

    gpio_init(GPIO_PIN(18), GPIO_OUT);
    flash_led(GPIO_PIN(18), 3);

	DEBUG("firmware version: %s\r\n", GIT_VERSION);

// 	cpuid_get(cpuid);
	DEBUG("cpuid: %02x %02x %02x %02x %02x %02x %02x %02x\n",
		   cpuid[0], cpuid[1], cpuid[2], cpuid[3], cpuid[4], cpuid[5], cpuid[6], cpuid[7]
	);
	cpuid[0] ^= cpuid[4]; // first byte is not sufficiently unique
	uint8_t radio_id = cpuid[0];
    DEBUG("radio_id: %02x\n", radio_id);

// 	setup_network(cpuid[0], false, 1111, &handle_packet, &handle_transmit);

//     gpio_init_int(BUTTON, GPIO_PULLDOWN, GPIO_RISING, button_handler, NULL);
//     gpio_init_int(BTN_1, GPIO_PULLDOWN, GPIO_RISING, button_handler, NULL);
//     gpio_init_int(BTN_2, GPIO_PULLDOWN, GPIO_RISING, button_handler, NULL);


}

void Sensortag::mainloop()
{
	thread_yield();

	DEBUG("starting mainloop...\r\n");

    xtimer_t t;
    int i =0;

    while(1) {
        printf("%i %lu %u\n", i++, xtimer_now() / 1000, timer_read(TIMER_DEV(0)));

        xtimer_set_wakeup(&t, 3000*1000, thread_getpid());
        thread_sleep();

        flash_led(GPIO_PIN(18), 2);
    }
}
