/*
 * Copyright (C) 2017 Eistec AB
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @defgroup    board_frdm-kw41z Freescale FRDM-KW41Z Board
 * @ingroup     boards
 * @brief       Board specific implementations for the FRDM-KW41Z
 * @{
 *
 * @file
 * @brief       Board specific definitions for the FRDM-KW41Z
 *
 * @author      Joakim Nohlgård <joakim.nohlgard@eistec.se>
 */

#ifndef BOARD_H
#define BOARD_H

#include "cpu.h"
#include "periph_conf.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief   LED pin definitions and handlers
 * @{
 */
#define LED0_PIN            GPIO_PIN(PORT_B,  0)
#define LED0_MASK           (1 << 0)
#define LED0_ON            (GPIOB->PCOR = LED0_MASK)
#define LED0_OFF           (GPIOB->PSOR = LED0_MASK)
#define LED0_TOGGLE        (GPIOB->PTOR = LED0_MASK)
/** @} */

/**
 * @brief   GPIO pin definitions
 * @{
 */
#define IO2_PIN            GPIO_PIN(PORT_A,  16)

/** @} */

/**
 * @brief   xtimer configuration
 * @{
 */
#if 1
/* LPTMR xtimer configuration */
/* WIP, Use PIT for now */
#define XTIMER_DEV                  (TIMER_LPTMR_DEV(0))
#define XTIMER_CHAN                 (0)
/* LPTMR is 16 bits wide */
#define XTIMER_WIDTH                (16)
#define XTIMER_BACKOFF              (5)
#define XTIMER_ISR_BACKOFF          (5)
#define XTIMER_OVERHEAD             (4)
#define XTIMER_HZ                   (32768ul)
#define XTIMER_SHIFT                (0)
#else
/* PIT xtimer configuration */
#define XTIMER_DEV                  (TIMER_PIT_DEV(0))
#define XTIMER_CHAN                 (0)
#define XTIMER_WIDTH                (32)
#define XTIMER_BACKOFF              (40)
#define XTIMER_ISR_BACKOFF          (40)
#define XTIMER_OVERHEAD             (30)
#define XTIMER_HZ                   (1000000ul)
#define XTIMER_SHIFT                (0)
#endif
/** @} */

/**
 * @name NOR flash hardware configuration
 */
/** @{ */
#define FRDM_NOR_SPI_DEV               SPI_DEV(0)
#define FRDM_NOR_SPI_CLK               SPI_CLK_5MHZ
#define FRDM_NOR_SPI_CS                SPI_HWCS(0) /**< Flash CS pin */
/** @} */

/**
 * @name MTD configuration
 */
/** @{ */
/* extern mtd_dev_t *mtd0; */
#define MTD_0 mtd0
/** @} */

/**
 * @brief Initialize board specific hardware, including clock, LEDs and std-IO
 */
void board_init(void);

#ifdef __cplusplus
}
#endif

#endif /** BOARD_H */
/** @} */