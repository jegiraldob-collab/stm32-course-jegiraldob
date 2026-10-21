/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Jeronimo Giraldo Betancur
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include <stdint.h>

#include "stm32f411xe.h"
#include "stm32f4xx.h"

volatile uint8_t bandera_tim11 = 0U;
volatile uint8_t flag_suma  = 0;
volatile uint8_t flag_resta = 0;
volatile uint16_t contador = 0; 
volatile uint8_t bandera_tim3 = 0U;

void led_ok_(void);
void tim11_Init(void);
void Display_Gpio_Init(void);
void Transistores_Init(void);
void Fotocompuerta(void);
void Encender_Segmentos(uint8_t digito);
void Actualizar_Display(void);
void apagar_todo(void);
void tim3_Init(void);

int main(void)
{
    led_ok_();
    tim11_Init();
    tim3_Init();
    Display_Gpio_Init();
    Transistores_Init();
    Fotocompuerta();
    contador = 0;



	while (1){

        if(flag_suma){
            flag_suma = 0U;
            contador++;
        }

        if(flag_resta){
            flag_resta = 0U;
            contador--;
        }
        
        if(bandera_tim3 == 1U){
            bandera_tim3 = 0U;
            Actualizar_Display();
        }




		if (bandera_tim11 == 1U)
		{
			bandera_tim11 = 0U;

			if ((GPIOH->ODR & GPIO_ODR_OD1) != 0U)
			{
				GPIOH->BSRR = GPIO_BSRR_BR1;
			}
			else
			{
				GPIOH->BSRR = GPIO_BSRR_BS1;
			}
		}
	}
}

/** Configura y apaga inicialmente el LED de la placa adicional. */
void led_ok_(void)
{
	/* Habilitar GPIOH y configurar PH1 (LED2 de expansion) como salida. */
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN;
	GPIOH->MODER &= ~GPIO_MODER_MODE1;
	GPIOH->MODER |= GPIO_MODER_MODE1_0;

	/* Iniciar LED_OK apagado. */
	GPIOH->BSRR = GPIO_BSRR_BR1;
}

/** Configura TIM11 para generar el parpadeo del LED de estado. */
void tim11_Init(void)
{
	/* TIM11 genera un evento cada 500 ms con reloj de 16 MHz. */
	RCC->APB2ENR |= RCC_APB2ENR_TIM11EN;
	TIM11->PSC = 15999U;
	TIM11->ARR = 499U;
	TIM11->CNT = 0U;

	/* TIM11 comparte su vector de interrupcion con TIM1. */
	TIM11->DIER |= TIM_DIER_UIE;
	NVIC_EnableIRQ(TIM1_TRG_COM_TIM11_IRQn);
	__enable_irq();

	/* CEN se activa al final de la configuracion. */
	TIM11->CR1 |= TIM_CR1_CEN;
}

/** Configura TIM3 para controlar la multiplexación del display. */
void tim3_Init(void){
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    TIM3->PSC = 15999U;
    TIM3->ARR = 3U;
    TIM3->CNT = 0U;

    TIM3->EGR = TIM_EGR_UG;
    TIM3->SR = 0U;
     TIM3->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM3_IRQn);

    TIM3->CR1 |= TIM_CR1_CEN;

}


/** Configura los GPIO que controlan los segmentos del display. */
void Display_Gpio_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN; //Activar señal de reloj GPIOC

    // PC5 controla el segmento G
    // PC6 controla el segmento C
    // PC8 controla el segmento D
    // PC9 controla el segmento E
    // PC10 controla el segmento B
    // PC11 controla el segmento A
    // PC12 controla el segmento F

    /* Segmento G: PC5 */
    GPIOC->MODER &= ~GPIO_MODER_MODE5;
    /* Segmento C: PC6 */
    GPIOC->MODER &= ~GPIO_MODER_MODE6;
    /* Segmento D: PC8 */
    GPIOC->MODER &= ~GPIO_MODER_MODE8;
    /* Segmento E: PC9 */
    GPIOC->MODER &= ~GPIO_MODER_MODE9;
    /* Segmento F: PC12 */
    GPIOC->MODER &= ~GPIO_MODER_MODE12;
    /* Segmento A: PC11 */
    GPIOC->MODER &= ~GPIO_MODER_MODE11;
    /* Segmento B: PC10 */
    GPIOC->MODER &= ~GPIO_MODER_MODE10;

    GPIOC->MODER |= GPIO_MODER_MODE5_0;
    GPIOC->MODER |= GPIO_MODER_MODE6_0;
    GPIOC->MODER |= GPIO_MODER_MODE8_0;
    GPIOC->MODER |= GPIO_MODER_MODE9_0;
    GPIOC->MODER |= GPIO_MODER_MODE12_0;
    GPIOC->MODER |= GPIO_MODER_MODE11_0;
    GPIOC->MODER |= GPIO_MODER_MODE10_0;

    GPIOC->BSRR = GPIO_BSRR_BR5;
    GPIOC->BSRR = GPIO_BSRR_BR6;
    GPIOC->BSRR = GPIO_BSRR_BR8;
    GPIOC->BSRR = GPIO_BSRR_BR9;
    GPIOC->BSRR = GPIO_BSRR_BR10;
    GPIOC->BSRR = GPIO_BSRR_BR11;
    GPIOC->BSRR = GPIO_BSRR_BR12;

    
}
/** Configura los GPIO que seleccionan cada dígito del display. */
void Transistores_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    // PB8 selecciona el dígito de unidades
    // PB9 selecciona el dígito de decenas
    // PB7 selecciona el dígito de centenas
    // PB0 selecciona el dígito de millares

    GPIOB->MODER &= ~GPIO_MODER_MODE0;
    GPIOB->MODER &= ~GPIO_MODER_MODE7;
    GPIOB->MODER &= ~GPIO_MODER_MODE8;
    GPIOB->MODER &= ~GPIO_MODER_MODE9;

    GPIOB->MODER |= GPIO_MODER_MODE0_0;
    GPIOB->MODER |= GPIO_MODER_MODE7_0;
    GPIOB->MODER |= GPIO_MODER_MODE8_0;
    GPIOB->MODER |= GPIO_MODER_MODE9_0;

    GPIOB->BSRR = GPIO_BSRR_BR0;
    GPIOB->BSRR = GPIO_BSRR_BR7;
    GPIOB->BSRR = GPIO_BSRR_BR8;
    GPIOB->BSRR = GPIO_BSRR_BR9;

}
/** Configura PA0 y PC4 como entradas de las fotocompuertas. */
void Fotocompuerta(void){
    // PA0 recibe la fotocompuerta de suma por flanco ascendente.
    // PC4 recibe la fotocompuerta de resta por flanco descendente.
    // Ambas entradas usan pull-up interno y deben compartir GND.
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

    GPIOA->MODER &= ~GPIO_MODER_MODE0;
    GPIOC->MODER &= ~GPIO_MODER_MODE4;
    
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD0;
    GPIOA->PUPDR |= GPIO_PUPDR_PUPD0_0;
    GPIOC->PUPDR &= ~GPIO_PUPDR_PUPD4;
    GPIOC->PUPDR |= GPIO_PUPDR_PUPD4_0;

    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI0;
    SYSCFG->EXTICR[1] &= ~SYSCFG_EXTICR2_EXTI4;
    SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI4_PC;
    
    EXTI->FTSR &= ~EXTI_FTSR_TR0;
    EXTI->FTSR |= EXTI_FTSR_TR4;
    EXTI->RTSR |= EXTI_RTSR_TR0;
    EXTI->RTSR &= ~EXTI_RTSR_TR4;
    EXTI->IMR |= (EXTI_IMR_MR0 | EXTI_IMR_MR4);
    EXTI->PR = (EXTI_PR_PR0 | EXTI_PR_PR4);

    NVIC_EnableIRQ(EXTI0_IRQn);
    NVIC_EnableIRQ(EXTI4_IRQn);
          
}
/** Atiende la interrupción de TIM11 y activa la bandera del LED. */
void TIM1_TRG_COM_TIM11_IRQHandler(void){
	if ((TIM11->SR & TIM_SR_UIF) != 0U)
	{
		TIM11->SR &= ~TIM_SR_UIF;
		bandera_tim11 = 1U;
	}
}
/** Enciende los segmentos necesarios para representar un dígito. */
void Encender_Segmentos(uint8_t digito){
    // El display es de ánodo común: un nivel bajo enciende el segmento.
    // Cada caso activa únicamente los segmentos necesarios para el dígito.
    switch(digito){
        case 0: /* A B C D E F - sin G*/
        GPIOC->BSRR = (GPIO_BSRR_BR11 |GPIO_BSRR_BR10 |GPIO_BSRR_BR6|
                        GPIO_BSRR_BR8|GPIO_BSRR_BR9|GPIO_BSRR_BR12);
        break;                
        case 1: /*B C*/
        GPIOC->BSRR =(GPIO_BSRR_BR10 | GPIO_BSRR_BR6);
        break;
        
        case 2: /* A B D E G — sin C F */
        GPIOC->BSRR = (GPIO_BSRR_BR11 | GPIO_BSRR_BR10 | GPIO_BSRR_BR8 |
                        GPIO_BSRR_BR9 | GPIO_BSRR_BR5);
                        break;
        case 3:   /* A B C D G — sin E F */
        GPIOC->BSRR = (GPIO_BSRR_BR11 | GPIO_BSRR_BR10 | GPIO_BSRR_BR6 |
                        GPIO_BSRR_BR8 | GPIO_BSRR_BR5);
                        break;
        case 4: /*B C F G — sin A D E*/
        GPIOC->BSRR = (GPIO_BSRR_BR10 | GPIO_BSRR_BR6 | GPIO_BSRR_BR12 |
                        GPIO_BSRR_BR5);
                        break;
        case 5: /* A C D F G — sin B E */
        GPIOC->BSRR = (GPIO_BSRR_BR11 | GPIO_BSRR_BR6 | GPIO_BSRR_BR8 | 
                        GPIO_BSRR_BR12 | GPIO_BSRR_BR5);
                        break;
        case 6: /*A C D E F G — sin B */
        GPIOC->BSRR = (GPIO_BSRR_BR11 | GPIO_BSRR_BR6 | GPIO_BSRR_BR8 |
                        GPIO_BSRR_BR9 | GPIO_BSRR_BR12 | GPIO_BSRR_BR5);
                        break;
        case 7: /*A B C — sin D E F G*/
        GPIOC->BSRR = (GPIO_BSRR_BR11 | GPIO_BSRR_BR10 | GPIO_BSRR_BR6);
                        break;
        case 8: /* A B C D E F G — todos */
        GPIOC->BSRR  = (GPIO_BSRR_BR11 | GPIO_BSRR_BR10 |
                   GPIO_BSRR_BR6  | GPIO_BSRR_BR8  |
                   GPIO_BSRR_BR9  | GPIO_BSRR_BR12 |
                   GPIO_BSRR_BR5);
                        break;
        case 9:
        GPIOC->BSRR = (GPIO_BSRR_BR11 | GPIO_BSRR_BR10 |
                   GPIO_BSRR_BR6  | GPIO_BSRR_BR8 | GPIO_BSRR_BR12 |
                   GPIO_BSRR_BR5);
                        break;
        default:
        /*Digito invalido: no encender ninguno*/
        break;                                                                                                                             
    }
}

/** Apaga los segmentos y desactiva todos los dígitos. */
void apagar_todo(void){
    // Apaga los cuatro dígitos antes de cambiar los segmentos.
    // Esto evita que aparezcan segmentos fantasma durante la multiplexación.
    GPIOB->BSRR=(GPIO_BSRR_BR0 | GPIO_BSRR_BR7 | GPIO_BSRR_BR8 |
                    GPIO_BSRR_BR9);
    GPIOC->BSRR=(GPIO_BSRR_BS12 | GPIO_BSRR_BS10 | GPIO_BSRR_BS6 |
                    GPIO_BSRR_BS8 | GPIO_BSRR_BS9 | GPIO_BSRR_BS11 | GPIO_BSRR_BS5 );


}


/** Multiplexa un dígito del display en cada llamada. */
void Actualizar_Display(void){
    // Actualiza un dígito por interrupción de TIM3.
    // La repetición rápida de los cuatro dígitos produce la imagen completa.
    static uint8_t digito_actual = 0;
    uint8_t millares = (contador/1000)%10;
    uint8_t centenas = (contador/100)%10;
    uint8_t decenas  = (contador/10)%10;
    uint8_t unidades = (contador)%10;

    apagar_todo();
    
    switch(digito_actual){
        case 0:
            Encender_Segmentos(unidades);
            GPIOB->BSRR = (GPIO_BSRR_BS8);
            break;
        case 1:
            Encender_Segmentos(decenas);
            GPIOB->BSRR = (GPIO_BSRR_BS9);
            break;
        case 2:
            Encender_Segmentos(centenas);
            GPIOB->BSRR = (GPIO_BSRR_BS7);
            break;
        case 3:
            Encender_Segmentos(millares);
            GPIOB->BSRR = (GPIO_BSRR_BS0);
            break;
        default:
            break;


    }
    if(++digito_actual>3){
        digito_actual =0;
    }


}

/** Atiende la fotocompuerta de suma conectada a PA0. */
void EXTI0_IRQHandler(void){
    // Señal de suma proveniente de PA0.
    if(EXTI->PR & EXTI_PR_PR0){
        EXTI->PR  = EXTI_PR_PR0;
        flag_suma = 1;
    }
}
/** Atiende la fotocompuerta de resta conectada a PC4. */
void EXTI4_IRQHandler(void){
    // Señal de resta proveniente de PC4.
    if(EXTI->PR & EXTI_PR_PR4){
        EXTI->PR = EXTI_PR_PR4;
        flag_resta = 1;
    }
}
/** Atiende TIM3 y solicita la actualización del display. */
void TIM3_IRQHandler(void)
{
    if ((TIM3->SR & TIM_SR_UIF) != 0U)
    {
        TIM3->SR &= ~TIM_SR_UIF;
        bandera_tim3 = 1U;
    }
}
