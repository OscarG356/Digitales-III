/**
 * @file    mcx153_wuu.h
 * @brief   Librería de hardware para el módulo WUU del microcontrolador MCX153 (NXP)
 * @author  Oscar Andrés Gutierrezs Rivadeneira
 * @date    2025-05-17
 *
 * @details Esta librería implementa el acceso a los registros del módulo Wake-Up Unit (WUU)
 *          del microcontrolador MCX153 de NXP.
 *
 *          La librería incluye:
 *          - Definición de tipos de datos para cada registro (uniones con estructuras de bits)
 *          - Máscaras para bits y campos de bits
 *          - Constantes para valores específicos de campos
 *          - Definición de la estructura completa del módulo
 *          - Macros de acceso corto a registros y campos de bits
 */


#ifndef MCX153_WUU_H
#define MCX153_WUU_H

#include <stdint.h>

/**
 * @brief Tipo de dato para el registro WUU_VERID
 * 
 * MAJOR:  Bits 31-24 Versión mayor
 * MINOR:  Bits 23-16 Versión menor
 * FEATURE:Bits 15-0  Características (0x0000 estándar, 0x0001 DMA/Trigger y detección extendida)
 */
typedef union
{
    uint32_t WORD;
    struct {
        uint32_t FEATURE : 16; ///< Bits 0-15: Características
        uint32_t MINOR   : 8;  ///< Bits 16-23: Versión menor
        uint32_t MAJOR   : 8;  ///< Bits 24-31: Versión mayor
    } BITS;
} __WUU_VERID_t;

/**
 * @brief Máscaras para WUU_VERID
 */
#define mWUU_FEATURE_VERID   (0x0000FFFFU)  ///< Bits 0-15: Características
#define mWUU_MINOR_VERID     (0x00FF0000U)  ///< Bits 16-23: Versión menor
#define mWUU_MAJOR_VERID     (0xFF000000U)  ///< Bits 24-31: Versión mayor

/**
 * @brief Tipo de dato para el registro WUU_PARAM
 */
typedef union
{
    uint32_t WORD;
    struct {
        uint32_t FILTERS : 8;  ///< Bits 0-7: Número de filtros de pin
        uint32_t DMAS    : 8;  ///< Bits 8-15: Número de fuentes DMA
        uint32_t MODULES : 8;  ///< Bits 16-23: Número de fuentes por módulo
        uint32_t PINS    : 8;  ///< Bits 24-31: Número de fuentes por pin
    } BITS;
} __WUU_PARAM_t;

/**
 * @brief Máscaras para los campos del registro WUU_PARAM
 */
#define mWUU_FILTERS_PARAM   (0x000000FFU)  ///< Bits 0-7: Número de filtros de pin
#define mWUU_DMAS_PARAM      (0x0000FF00U)  ///< Bits 8-15: Número de fuentes DMA
#define mWUU_MODULES_PARAM   (0x00FF0000U)  ///< Bits 16-23: Número de fuentes por módulo
#define mWUU_PINS_PARAM      (0xFF000000U)  ///< Bits 24-31: Número de fuentes por pin

/**
 * @brief Tipo de dato para el registro WUU_PE1
 */
/**
 * @brief Tipo de dato para el registro WUU_PE1
 * 
 * Este registro controla la habilitación y la detección de flancos/niveles para los pines de wake-up.
 * Los campos WUPE[n] permiten seleccionar el tipo de detección para cada pin:
 *  00b - Deshabilitado
 *  01b - Habilitado (detecta flanco ascendente o nivel alto)
 *  10b - Habilitado (detecta flanco descendente o nivel bajo)
 *  11b - Habilitado (detecta cualquier flanco)
 * @note Los campos reservados no deben ser modificados.
 */
typedef union
{
    uint32_t WORD;  ///< Acceso completo al registro
    struct {
        uint32_t        : 2;  ///< Bits 0-1: Reservados
        uint32_t        : 2;  ///< Bits 2-3: Reservados
        uint32_t WUPE2  : 2;  ///< Bits 4-5: Habilitación y tipo de detección para el pin 2
        uint32_t        : 2;  ///< Bits 6-7: Reservados
        uint32_t        : 2;  ///< Bits 8-9: Reservados
        uint32_t        : 2;  ///< Bits 10-11: Reservados
        uint32_t WUPE6  : 2;  ///< Bits 12-13: Habilitación y tipo de detección para el pin 6
        uint32_t WUPE7  : 2;  ///< Bits 14-15: Habilitación y tipo de detección para el pin 7
        uint32_t WUPE8  : 2;  ///< Bits 16-17: Habilitación y tipo de detección para el pin 8
        uint32_t WUPE9  : 2;  ///< Bits 18-19: Habilitación y tipo de detección para el pin 9
        uint32_t WUPE10 : 2;  ///< Bits 20-21: Habilitación y tipo de detección para el pin 10
        uint32_t WUPE11 : 2;  ///< Bits 22-23: Habilitación y tipo de detección para el pin 11
        uint32_t WUPE12 : 2;  ///< Bits 24-25: Habilitación y tipo de detección para el pin 12
        uint32_t        : 2;  ///< Bits 26-27: Reservados
        uint32_t        : 2;  ///< Bits 28-29: Reservados
        uint32_t        : 2;  ///< Bits 30-31: Reservados
    }BITS;
} __WUU_PE1_t;

/**
 * @brief Máscaras para los campos del registro WUU_PE1
 */
#define mWUU_PE1_WUPE2    (0x00000030U)  ///< Bits 4-5
#define mWUU_PE1_WUPE6    (0x00003000U)  ///< Bits 12-13
#define mWUU_PE1_WUPE7    (0x0000C000U)  ///< Bits 14-15
#define mWUU_PE1_WUPE8    (0x00030000U)  ///< Bits 16-17
#define mWUU_PE1_WUPE9    (0x000C0000U)  ///< Bits 18-19
#define mWUU_PE1_WUPE10   (0x00300000U)  ///< Bits 20-21
#define mWUU_PE1_WUPE11   (0x00C00000U)  ///< Bits 22-23
#define mWUU_PE1_WUPE12   (0x03000000U)  ///< Bits 24-25

/**
 * @brief Constantes para los campos WUPE[n] de WUU_PE1.
 */
#define kWUU_WUPE_DISABLED     (0x0U) ///< 00b - Deshabilitado
#define kWUU_WUPE_RISING       (0x1U) ///< 01b - Flanco ascendente o nivel alto
#define kWUU_WUPE_FALLING      (0x2U) ///< 10b - Flanco descendente o nivel bajo
#define kWUU_WUPE_ANYEDGE      (0x3U) ///< 11b - Cualquier flanco


/**
 * @brief Tipo de dato para el registro WUU_PE2
 * 
 * Este registro controla la habilitación y la detección de flancos/niveles para los pines de wake-up 18-28.
 * Los campos WUPE[n] permiten seleccionar el tipo de detección para cada pin:
 *  00b - Deshabilitado
 *  01b - Habilitado (detecta flanco ascendente o nivel alto)
 *  10b - Habilitado (detecta flanco descendente o nivel bajo)
 *  11b - Habilitado (detecta cualquier flanco)
 * @note Los campos reservados no deben ser modificados.
 */
typedef union
{
    uint32_t WORD;  ///< Acceso completo al registro
    struct {
        uint32_t        : 2;  ///< Bits 0-1: Reservados
        uint32_t        : 2;  ///< Bits 2-3: Reservados
        uint32_t WUPE18 : 2;  ///< Bits 4-5: Habilitación y tipo de detección para el pin 18
        uint32_t WUPE19 : 2;  ///< Bits 6-7: Habilitación y tipo de detección para el pin 19
        uint32_t WUPE20 : 2;  ///< Bits 8-9: Habilitación y tipo de detección para el pin 20
        uint32_t        : 2;  ///< Bits 10-11: Reservados
        uint32_t WUPE22 : 2;  ///< Bits 12-13: Habilitación y tipo de detección para el pin 22
        uint32_t WUPE23 : 2;  ///< Bits 14-15: Habilitación y tipo de detección para el pin 23
        uint32_t WUPE24 : 2;  ///< Bits 16-17: Habilitación y tipo de detección para el pin 24
        uint32_t WUPE25 : 2;  ///< Bits 18-19: Habilitación y tipo de detección para el pin 25
        uint32_t WUPE26 : 2;  ///< Bits 20-21: Habilitación y tipo de detección para el pin 26
        uint32_t WUPE27 : 2;  ///< Bits 22-23: Habilitación y tipo de detección para el pin 27
        uint32_t WUPE28 : 2;  ///< Bits 24-25: Habilitación y tipo de detección para el pin 28
        uint32_t        : 6;  ///< Bits 26-31: Reservados
    }BITS;
} __WUU_PE2_t;

/**
 * @brief Máscaras para los campos del registro WUU_PE2
 */
#define mWUU_PE2_WUPE18   (0x00000030U)  ///< Bits 4-5
#define mWUU_PE2_WUPE19   (0x000000C0U)  ///< Bits 6-7
#define mWUU_PE2_WUPE20   (0x00000300U)  ///< Bits 8-9
#define mWUU_PE2_WUPE22   (0x00003000U)  ///< Bits 12-13
#define mWUU_PE2_WUPE23   (0x0000C000U)  ///< Bits 14-15
#define mWUU_PE2_WUPE24   (0x00030000U)  ///< Bits 16-17
#define mWUU_PE2_WUPE25   (0x000C0000U)  ///< Bits 18-19
#define mWUU_PE2_WUPE26   (0x00300000U)  ///< Bits 20-21
#define mWUU_PE2_WUPE27   (0x00C00000U)  ///< Bits 22-23
#define mWUU_PE2_WUPE28   (0x03000000U)  ///< Bits 24-25

/**
 * @brief Constantes para los campos WUPE[n] de WUU_PE2.
 */
#define kWUU_WUPE_DISABLED     (0x0U) ///< 00b - Deshabilitado
#define kWUU_WUPE_RISING       (0x1U) ///< 01b - Flanco ascendente o nivel alto
#define kWUU_WUPE_FALLING      (0x2U) ///< 10b - Flanco descendente o nivel bajo
#define kWUU_WUPE_ANYEDGE      (0x3U) ///< 11b - Cualquier flanco


/**
 * @brief Tipo de dato para el registro WUU_ME
 * 
 * Este registro controla la habilitación de las fuentes de interrupción de módulo como fuentes de wake-up.
 * 
 * Campos:
 *  - WUME0  (bit 0):  Habilita fuente de módulo 0
 *  - WUME2  (bit 2):  Habilita fuente de módulo 2
 *  - WUME6  (bit 6):  Habilita fuente de módulo 6
 *  - WUME8  (bit 8):  Habilita fuente de módulo 8
 *  @note Los demás bits son reservados y no deben ser modificados.
 */
typedef union
{
    uint32_t WORD;
    struct {
        uint32_t WUME0  : 1;  ///< Bit 0: Habilitación para módulo 0
        uint32_t        : 1;  ///< Bit 1: Reservado
        uint32_t WUME2  : 1;  ///< Bit 2: Habilitación para módulo 2
        uint32_t        : 2;  ///< Bits 3-4: Reservados
        uint32_t        : 1;  ///< Bit 5: Reservado
        uint32_t WUME6  : 1;  ///< Bit 6: Habilitación para módulo 6
        uint32_t        : 1;  ///< Bit 7: Reservado
        uint32_t WUME8  : 1;  ///< Bit 8: Habilitación para módulo 8
        uint32_t        : 23; ///< Bits 9-31: Reservados
    } BITS;
} __WUU_ME_t;

/**
 * @brief Máscaras para los campos del registro WUU_ME
 */
#define mWUU_ME_WUME0   (0x00000001U) ///< Bit 0
#define mWUU_ME_WUME2   (0x00000004U) ///< Bit 2
#define mWUU_ME_WUME6   (0x00000040U) ///< Bit 6
#define mWUU_ME_WUME8   (0x00000100U) ///< Bit 8

/**
 * @brief Tipo de dato para el registro WUU_DE (DMA/Trigger Enable)
 * 
 * Este registro controla la habilitación de las fuentes DMA/Trigger de los módulos como fuentes de wake-up.
 * 
 * Campos:
 *  - WUDE4  (bit 4):  Habilita DMA/Trigger para módulo 4
 *  - WUDE6  (bit 6):  Habilita DMA/Trigger para módulo 6
 *  - WUDE8  (bit 8):  Habilita DMA/Trigger para módulo 8
 *  @note Los demás bits son reservados y no deben ser modificados.
 */
typedef union
{
    uint32_t WORD;
    struct {
        uint32_t        : 4;  ///< Bits 0-3: Reservados
        uint32_t WUDE4  : 1;  ///< Bit 4: Habilitación DMA/Trigger para módulo 4
        uint32_t        : 1;  ///< Bit 5: Reservado
        uint32_t WUDE6  : 1;  ///< Bit 6: Habilitación DMA/Trigger para módulo 6
        uint32_t        : 1;  ///< Bit 7: Reservado
        uint32_t WUDE8  : 1;  ///< Bit 8: Habilitación DMA/Trigger para módulo 8
        uint32_t        : 23; ///< Bits 9-31: Reservados
    } BITS;
} __WUU_DE_t;

/**
 * @brief Máscaras para los campos del registro WUU_DE
 */
#define mWUU_DE_WUDE4   (0x00000010U) ///< Bit 4
#define mWUU_DE_WUDE6   (0x00000040U) ///< Bit 6
#define mWUU_DE_WUDE8   (0x00000100U) ///< Bit 8

/**
 * @brief Tipo de dato para el registro WUU_PF (Pin Flag)
 * 
 * Este registro indica qué pines de wake-up han causado la salida de un modo de bajo consumo.
 * Los campos WUF[n] indican el estado de cada pin:
 *  0b - No ha causado salida
 *  1b - Sí ha causado salida
 * @note Los campos reservados no deben ser modificados.
 */
typedef union
{
    uint32_t WORD;
    struct {
        uint32_t        : 2;   ///< Bits 0-1: Reservados
        uint32_t WUF2   : 1;   ///< Bit 2: Wake-up Flag para pin 2
        uint32_t        : 3;   ///< Bits 3-5: Reservados
        uint32_t WUF6   : 1;   ///< Bit 6: Wake-up Flag para pin 6
        uint32_t WUF7   : 1;   ///< Bit 7: Wake-up Flag para pin 7
        uint32_t WUF8   : 1;   ///< Bit 8: Wake-up Flag para pin 8
        uint32_t WUF9   : 1;   ///< Bit 9: Wake-up Flag para pin 9
        uint32_t WUF10  : 1;   ///< Bit 10: Wake-up Flag para pin 10
        uint32_t WUF11  : 1;   ///< Bit 11: Wake-up Flag para pin 11
        uint32_t WUF12  : 1;   ///< Bit 12: Wake-up Flag para pin 12
        uint32_t        : 3;   ///< Bits 13-15: Reservados
        uint32_t        : 1;   ///< Bit 16: Reservado
        uint32_t        : 1;   ///< Bit 17: Reservado
        uint32_t WUF18  : 1;   ///< Bit 18: Wake-up Flag para pin 18
        uint32_t WUF19  : 1;   ///< Bit 19: Wake-up Flag para pin 19
        uint32_t WUF20  : 1;   ///< Bit 20: Wake-up Flag para pin 20
        uint32_t        : 1;   ///< Bit 21: Reservado
        uint32_t WUF22  : 1;   ///< Bit 22: Wake-up Flag para pin 22
        uint32_t WUF23  : 1;   ///< Bit 23: Wake-up Flag para pin 23
        uint32_t WUF24  : 1;   ///< Bit 24: Wake-up Flag para pin 24
        uint32_t WUF25  : 1;   ///< Bit 25: Wake-up Flag para pin 25
        uint32_t WUF26  : 1;   ///< Bit 26: Wake-up Flag para pin 26
        uint32_t WUF27  : 1;   ///< Bit 27: Wake-up Flag para pin 27
        uint32_t WUF28  : 1;   ///< Bit 28: Wake-up Flag para pin 28
        uint32_t        : 3;   ///< Bits 29-31: Reservados
    } BITS;
} __WUU_PF_t;

/**
 * @brief Máscaras para los campos del registro WUU_PF
 */
#define mWUU_PF_WUF2    (0x00000004U)  ///< Bit 2
#define mWUU_PF_WUF6    (0x00000040U)  ///< Bit 6
#define mWUU_PF_WUF7    (0x00000080U)  ///< Bit 7
#define mWUU_PF_WUF8    (0x00000100U)  ///< Bit 8
#define mWUU_PF_WUF9    (0x00000200U)  ///< Bit 9
#define mWUU_PF_WUF10   (0x00000400U)  ///< Bit 10
#define mWUU_PF_WUF11   (0x00000800U)  ///< Bit 11
#define mWUU_PF_WUF12   (0x00001000U)  ///< Bit 12
#define mWUU_PF_WUF18   (0x00040000U)  ///< Bit 18
#define mWUU_PF_WUF19   (0x00080000U)  ///< Bit 19
#define mWUU_PF_WUF20   (0x00100000U)  ///< Bit 20
#define mWUU_PF_WUF22   (0x00400000U)  ///< Bit 22
#define mWUU_PF_WUF23   (0x00800000U)  ///< Bit 23
#define mWUU_PF_WUF24   (0x01000000U)  ///< Bit 24
#define mWUU_PF_WUF25   (0x02000000U)  ///< Bit 25
#define mWUU_PF_WUF26   (0x04000000U)  ///< Bit 26
#define mWUU_PF_WUF27   (0x08000000U)  ///< Bit 27
#define mWUU_PF_WUF28   (0x10000000U)  ///< Bit 28

/**
 * @brief Tipo de dato para el registro WUU_FILT (Pin Filter)
 * 
 * Este registro controla los filtros de pines de wake-up y su estado.
 */
typedef union
{
    uint32_t WORD;
    struct {
        uint32_t FILTSEL1 : 5;  ///< Bits 0-4: Selección de pin para filtro 1
        uint32_t FILTE1   : 2;  ///< Bits 5-6: Habilitación y tipo de detección para filtro 1
        uint32_t FILTF1   : 1;  ///< Bit 7: Bandera de evento de filtro 1
        uint32_t FILTSEL2 : 5;  ///< Bits 8-12: Selección de pin para filtro 2
        uint32_t FILTE2   : 2;  ///< Bits 13-14: Habilitación y tipo de detección para filtro 2
        uint32_t FILTF2   : 1;  ///< Bit 15: Bandera de evento de filtro 2
        uint32_t          : 16; ///< Bits 16-31: Reservados
    } BITS;
} __WUU_FILT_t;

/**
 * @brief Constantes para los campos FILTE[n] del registro WUU_FILT
 */
#define kWUU_FILTE_DISABLED     (0x0U) ///< 00b - Deshabilitado
#define kWUU_FILTE_RISING       (0x1U) ///< 01b - Flanco ascendente
#define kWUU_FILTE_FALLING      (0x2U) ///< 10b - Flanco descendente
#define kWUU_FILTE_ANYEDGE      (0x3U) ///< 11b - Cualquier flanco


/**
 * @brief Máscaras para los campos del registro WUU_FILT
 */
#define mWUU_FILT_FILTSEL1   (0x0000001FU)  ///< Bits 0-4: Selección de pin para filtro 1
#define mWUU_FILT_FILTE1     (0x00000060U)  ///< Bits 5-6: Habilitación y tipo de detección para filtro 1
#define mWUU_FILT_FILTF1     (0x00000080U)  ///< Bit 7: Bandera de evento de filtro 1
#define mWUU_FILT_FILTSEL2   (0x00001F00U)  ///< Bits 8-12: Selección de pin para filtro 2
#define mWUU_FILT_FILTE2     (0x00006000U)  ///< Bits 13-14: Habilitación y tipo de detección para filtro 2
#define mWUU_FILT_FILTF2     (0x00008000U)  ///< Bit 15: Bandera de evento de filtro 2

/**
 * @brief Tipo de dato para el registro WUU_PDC1 (Pin DMA/Trigger Configuration 1)
 * 
 * Este registro configura cada pin de wake-up como fuente de interrupción, DMA o trigger.
 * 
 * Valores posibles para cada campo WUPDCn:
 *  00b - Interrupt
 *  01b - DMA request
 *  10b - Trigger event
 *  11b - Reserved
 */
typedef union
{
    uint32_t WORD;
    struct {
        uint32_t        : 2;  ///< Bits 0-1: Reservados
        uint32_t        : 2;  ///< Bits 2-3: Reservados
        uint32_t WUPDC2 : 2;  ///< Bits 4-5: Configuración para pin 2
        uint32_t        : 6;  ///< Bits 6-11: Reservados
        uint32_t WUPDC6 : 2;  ///< Bits 12-13: Configuración para pin 6
        uint32_t WUPDC7 : 2;  ///< Bits 14-15: Configuración para pin 7
        uint32_t WUPDC8 : 2;  ///< Bits 16-17: Configuración para pin 8
        uint32_t WUPDC9 : 2;  ///< Bits 18-19: Configuración para pin 9
        uint32_t WUPDC10: 2;  ///< Bits 20-21: Configuración para pin 10
        uint32_t WUPDC11: 2;  ///< Bits 22-23: Configuración para pin 11
        uint32_t WUPDC12: 2;  ///< Bits 24-25: Configuración para pin 12
        uint32_t        : 6;  ///< Bits 26-31: Reservados
    } BITS;
} __WUU_PDC1_t;

/**
 * @brief Máscaras para los campos del registro WUU_PDC1
 */
#define mWUU_PDC1_WUPDC2    (0x00000030U)  ///< Bits 4-5
#define mWUU_PDC1_WUPDC6    (0x00003000U)  ///< Bits 12-13
#define mWUU_PDC1_WUPDC7    (0x0000C000U)  ///< Bits 14-15
#define mWUU_PDC1_WUPDC8    (0x00030000U)  ///< Bits 16-17
#define mWUU_PDC1_WUPDC9    (0x000C0000U)  ///< Bits 18-19
#define mWUU_PDC1_WUPDC10   (0x00300000U)  ///< Bits 20-21
#define mWUU_PDC1_WUPDC11   (0x00C00000U)  ///< Bits 22-23
#define mWUU_PDC1_WUPDC12   (0x03000000U)  ///< Bits 24-25

/**
 * @brief Constantes para los campos WUPDC[n] del registro WUU_PDC1
 */
#define kWUU_WUPDC_INTERRUPT_PDC1   (0x0U) ///< 00b - Interrupt
#define kWUU_WUPDC_DMA_PDC1         (0x1U) ///< 01b - DMA request
#define kWUU_WUPDC_TRIGGER_PDC1     (0x2U) ///< 10b - Trigger event
#define kWUU_WUPDC_RESERVED_PDC1    (0x3U) ///< 11b - Reservado, no usar

/**
 * @brief Tipo de dato para el registro WUU_PDC2 (Pin DMA/Trigger Configuration 2)
 * 
 * Este registro configura cada pin de wake-up (18,19,20,22,23,24,25,26,27,28,29,31) como fuente de interrupción, DMA o trigger.
 * 
 * @note Los campos reservados no deben ser modificados.
 */
typedef union
{
    uint32_t WORD;
    struct {
        uint32_t        : 2;  ///< Bits 0-1: Reservados
        uint32_t        : 2;  ///< Bits 2-3: Reservados
        uint32_t WUPDC18: 2;  ///< Bits 4-5: Configuración para pin 18
        uint32_t WUPDC19: 2;  ///< Bits 6-7: Configuración para pin 19
        uint32_t WUPDC20: 2;  ///< Bits 8-9: Configuración para pin 20
        uint32_t        : 2;  ///< Bits 10-11: Reservados
        uint32_t WUPDC22: 2;  ///< Bits 12-13: Configuración para pin 22
        uint32_t WUPDC23: 2;  ///< Bits 14-15: Configuración para pin 23
        uint32_t WUPDC24: 2;  ///< Bits 16-17: Configuración para pin 24
        uint32_t WUPDC25: 2;  ///< Bits 18-19: Configuración para pin 25
        uint32_t WUPDC26: 2;  ///< Bits 20-21: Configuración para pin 26
        uint32_t WUPDC27: 2;  ///< Bits 22-23: Configuración para pin 27
        uint32_t WUPDC28: 2;  ///< Bits 24-25: Configuración para pin 28
        uint32_t WUPDC29: 2;  ///< Bits 26-27: Configuración para pin 29
        uint32_t        : 2;  ///< Bits 28-29: Reservados
        uint32_t WUPDC31: 2;  ///< Bits 30-31: Configuración para pin 31
    } BITS;
} __WUU_PDC2_t;

/**
 * @brief Máscaras para los campos del registro WUU_PDC2
 */
#define mWUU_PDC2_WUPDC18   (0x00000030U)  ///< Bits 4-5
#define mWUU_PDC2_WUPDC19   (0x000000C0U)  ///< Bits 6-7
#define mWUU_PDC2_WUPDC20   (0x00000300U)  ///< Bits 8-9
#define mWUU_PDC2_WUPDC22   (0x00003000U)  ///< Bits 12-13
#define mWUU_PDC2_WUPDC23   (0x0000C000U)  ///< Bits 14-15
#define mWUU_PDC2_WUPDC24   (0x00030000U)  ///< Bits 16-17
#define mWUU_PDC2_WUPDC25   (0x000C0000U)  ///< Bits 18-19
#define mWUU_PDC2_WUPDC26   (0x00300000U)  ///< Bits 20-21
#define mWUU_PDC2_WUPDC27   (0x00C00000U)  ///< Bits 22-23
#define mWUU_PDC2_WUPDC28   (0x03000000U)  ///< Bits 24-25
#define mWUU_PDC2_WUPDC29   (0x0C000000U)  ///< Bits 26-27
#define mWUU_PDC2_WUPDC31   (0xC0000000U)  ///< Bits 30-31

/**
 * @brief Constantes para los campos WUPDC[n] del registro WUU_PDC2
 */
#define kWUU_WUPDC_INTERRUPT_PDC2   (0x0U) ///< 00b - Interrupt
#define kWUU_WUPDC_DMA_PDC2         (0x1U) ///< 01b - DMA request
#define kWUU_WUPDC_TRIGGER_PDC2     (0x2U) ///< 10b - Trigger event
#define kWUU_WUPDC_RESERVED_PDC2    (0x3U) ///< 11b - Reservado, no usar


/**
 * @brief Tipo de dato para el registro WUU_FDC (Filter DMA/Trigger Configuration)
 *
 * Este registro configura cada filtro de wake-up como fuente de interrupción, DMA o trigger.
 */
typedef union
{
    uint32_t WORD;
    struct {
        uint32_t FILTC1 : 2;  ///< Bits 0-1: Configuración para filtro 1
        uint32_t FILTC2 : 2;  ///< Bits 2-3: Configuración para filtro 2
        uint32_t        : 28; ///< Bits 4-31: Reservados
    } BITS;
} __WUU_FDC_t;

/**
 * @brief Máscaras para los campos del registro WUU_FDC
 */
#define mWUU_FDC_FILTC1   (0x00000003U) ///< Bits 0-1
#define mWUU_FDC_FILTC2   (0x0000000CU) ///< Bits 2-3

/**
 * @brief Constantes para los campos FILTC[n] del registro WUU_FDC
 */
#define kWUU_FILTC_INTERRUPT   (0x0U) ///< 00b - Interrupt
#define kWUU_FILTC_DMA         (0x1U) ///< 01b - DMA request
#define kWUU_FILTC_TRIGGER     (0x2U) ///< 10b - Trigger event
#define kWUU_FILTC_RESERVED    (0x3U) ///< 11b - Reservado, no usar

/**
 * @union __WUU_PMC_t
 * @brief Unión para el registro WUU_PMC (Pin Mode Configuration).
 *
 * Permite configurar el modo de activación de cada pin de wake-up.
 * Los bits válidos (WUPMC[n]) determinan si el pin está activo en 
 * todos los modos de bajo consumo o solo en modo de espera.
 *
 * @note No modificar los bits reservados.
 */

typedef union
{
    uint32_t WORD;
    struct {
        uint32_t        : 2;   ///< Bits 0-1: Reservados
        uint32_t WUPMC2 : 1;   ///< Bit 2: Configuración de modo para pin 2
        uint32_t        : 3;   ///< Bits 3-5: Reservados
        uint32_t WUPMC6 : 1;   ///< Bit 6: Configuración de modo para pin 6
        uint32_t WUPMC7 : 1;   ///< Bit 7: Configuración de modo para pin 7
        uint32_t WUPMC8 : 1;   ///< Bit 8: Configuración de modo para pin 8
        uint32_t WUPMC9 : 1;   ///< Bit 9: Configuración de modo para pin 9
        uint32_t WUPMC10: 1;   ///< Bit 10: Configuración de modo para pin 10
        uint32_t WUPMC11: 1;   ///< Bit 11: Configuración de modo para pin 11
        uint32_t WUPMC12: 1;   ///< Bit 12: Configuración de modo para pin 12
        uint32_t        : 4;   ///< Bits 13-16: Reservados
        uint32_t        : 1;   ///< Bit 17: Reservado
        uint32_t WUPMC18: 1;   ///< Bit 18: Configuración de modo para pin 18
        uint32_t WUPMC19: 1;   ///< Bit 19: Configuración de modo para pin 19
        uint32_t WUPMC20: 1;   ///< Bit 20: Configuración de modo para pin 20
        uint32_t        : 1;   ///< Bit 21: Reservado
        uint32_t WUPMC22: 1;   ///< Bit 22: Configuración de modo para pin 22
        uint32_t WUPMC23: 1;   ///< Bit 23: Configuración de modo para pin 23
        uint32_t WUPMC24: 1;   ///< Bit 24: Configuración de modo para pin 24
        uint32_t WUPMC25: 1;   ///< Bit 25: Configuración de modo para pin 25
        uint32_t WUPMC26: 1;   ///< Bit 26: Configuración de modo para pin 26
        uint32_t WUPMC27: 1;   ///< Bit 27: Configuración de modo para pin 27
        uint32_t WUPMC28: 1;   ///< Bit 28: Configuración de modo para pin 28
        uint32_t WUPMC29: 1;   ///< Bit 29: Configuración de modo para pin 29
        uint32_t        : 1;   ///< Bit 30: Reservado
        uint32_t WUPMC31: 1;   ///< Bit 31: Configuración de modo para pin 31
    } BITS;
} __WUU_PMC_t;

/**
 * @brief Máscaras para los campos del registro WUU_PMC
 */
#define mWUU_PMC_WUPMC2    (0x00000004U)  ///< Bit 2
#define mWUU_PMC_WUPMC6    (0x00000040U)  ///< Bit 6
#define mWUU_PMC_WUPMC7    (0x00000080U)  ///< Bit 7
#define mWUU_PMC_WUPMC8    (0x00000100U)  ///< Bit 8
#define mWUU_PMC_WUPMC9    (0x00000200U)  ///< Bit 9
#define mWUU_PMC_WUPMC10   (0x00000400U)  ///< Bit 10
#define mWUU_PMC_WUPMC11   (0x00000800U)  ///< Bit 11
#define mWUU_PMC_WUPMC12   (0x00001000U)  ///< Bit 12
#define mWUU_PMC_WUPMC18   (0x00040000U)  ///< Bit 18
#define mWUU_PMC_WUPMC19   (0x00080000U)  ///< Bit 19
#define mWUU_PMC_WUPMC20   (0x00100000U)  ///< Bit 20
#define mWUU_PMC_WUPMC22   (0x00400000U)  ///< Bit 22
#define mWUU_PMC_WUPMC23   (0x00800000U)  ///< Bit 23
#define mWUU_PMC_WUPMC24   (0x01000000U)  ///< Bit 24
#define mWUU_PMC_WUPMC25   (0x02000000U)  ///< Bit 25
#define mWUU_PMC_WUPMC26   (0x04000000U)  ///< Bit 26
#define mWUU_PMC_WUPMC27   (0x08000000U)  ///< Bit 27
#define mWUU_PMC_WUPMC28   (0x10000000U)  ///< Bit 28
#define mWUU_PMC_WUPMC29   (0x20000000U)  ///< Bit 29
#define mWUU_PMC_WUPMC31   (0x80000000U)  ///< Bit 31

/**
 * @brief Constantes para los campos WUPMC[n] del registro WUU_PMC
 */
#define kWUU_WUPMC_LOWLEAKAGE   (0x0U) ///< 0b - Activo solo en modo de bajo consumo
#define kWUU_WUPMC_ALLMODES     (0x1U) ///< 1b - Activo en todos los modos de energía

/**
 * @brief Tipo de dato para el registro WUU_FMC (Filter Mode Configuration)
 *
 * Este registro configura el modo de activación de los filtros de pines de wake-up para todos los modos de energía.
 */
typedef union
{
    uint32_t WORD;
    struct {
        uint32_t FILTM1 : 1;   ///< Bit 0: Configuración de modo para filtro 1
        uint32_t FILTM2 : 1;   ///< Bit 1: Configuración de modo para filtro 2
        uint32_t        : 30;  ///< Bits 2-31: Reservados
    } BITS;
} __WUU_FMC_t;

/**
 * @brief Máscaras para los campos del registro WUU_FMC
 */
#define mWUU_FMC_FILTM1    (0x00000001U)  ///< Bit 0
#define mWUU_FMC_FILTM2    (0x00000002U)  ///< Bit 1

/**
 * @brief Constantes para los campos FILTM[n] del registro WUU_FMC
 */
#define kWUU_FILTM_LOWLEAKAGE   (0x0U) ///< 0b - Activo solo en modo de bajo consumo
#define kWUU_FILTM_ALLMODES     (0x1U) ///< 1b - Activo en todos los modos de energía


/**
 * @brief Mapa completo de registros del módulo WUU (offsets en bytes)
 */
typedef struct __WUU__t {
    volatile __WUU_VERID_t VERID; // 0x00
    volatile __WUU_PARAM_t PARAM; // 0x04
    volatile __WUU_PE1_t   PE1;   // 0x08
    volatile __WUU_PE2_t   PE2;   // 0x0C
    uint32_t _reserved0[2];       // 0x10, 0x14
    volatile __WUU_ME_t    ME;    // 0x18
    volatile __WUU_DE_t    DE;    // 0x1C
    volatile __WUU_PF_t    PF;    // 0x20
    uint32_t _reserved1[3];       // 0x24–0x2C
    volatile __WUU_FILT_t  FILT;  // 0x30
    uint32_t _reserved2;          // 0x34
    volatile __WUU_PDC1_t  PDC1;  // 0x38
    volatile __WUU_PDC2_t  PDC2;  // 0x3C
    uint32_t _reserved3[2];       // 0x40–0x44
    volatile __WUU_FDC_t   FDC;   // 0x48
    uint32_t _reserved4;          // 0x4C
    volatile __WUU_PMC_t   PMC;   // 0x50
    uint32_t _reserved5;          // 0x54
    volatile __WUU_FMC_t   FMC;   // 0x58
} __WUU__t;


/**  
 * @brief Puntero al bloque WUU0 en memoria  
 */  
#define sWUU0   ((volatile __WUU__t*)0x4007F000U)


/* === Accesos rápidos al registro WUU_VERID === */
#define bWUU0_FEATURE     (sWUU0->VERID.BITS.FEATURE)
#define bWUU0_MINOR       (sWUU0->VERID.BITS.MINOR)
#define bWUU0_MAJOR       (sWUU0->VERID.BITS.MAJOR)

/* === Accesos rápidos al registro WUU_PARAM === */
#define bWUU0_FILTERS     (sWUU0->PARAM.BITS.FILTERS)
#define bWUU0_DMAS        (sWUU0->PARAM.BITS.DMAS)
#define bWUU0_MODULES     (sWUU0->PARAM.BITS.MODULES)
#define bWUU0_PINS        (sWUU0->PARAM.BITS.PINS)

/* === Accesos rápidos al registro WUU_PE1 === */
#define bWUU0_WUPE2       (sWUU0->PE1.BITS.WUPE2)
#define bWUU0_WUPE6       (sWUU0->PE1.BITS.WUPE6)
#define bWUU0_WUPE7       (sWUU0->PE1.BITS.WUPE7)
#define bWUU0_WUPE8       (sWUU0->PE1.BITS.WUPE8)
#define bWUU0_WUPE9       (sWUU0->PE1.BITS.WUPE9)
#define bWUU0_WUPE10      (sWUU0->PE1.BITS.WUPE10)
#define bWUU0_WUPE11      (sWUU0->PE1.BITS.WUPE11)
#define bWUU0_WUPE12      (sWUU0->PE1.BITS.WUPE12)

/* === Accesos rápidos al registro WUU_PE2 === */
#define bWUU0_WUPE18      (sWUU0->PE2.BITS.WUPE18)
#define bWUU0_WUPE19      (sWUU0->PE2.BITS.WUPE19)
#define bWUU0_WUPE20      (sWUU0->PE2.BITS.WUPE20)
#define bWUU0_WUPE22      (sWUU0->PE2.BITS.WUPE22)
#define bWUU0_WUPE23      (sWUU0->PE2.BITS.WUPE23)
#define bWUU0_WUPE24      (sWUU0->PE2.BITS.WUPE24)
#define bWUU0_WUPE25      (sWUU0->PE2.BITS.WUPE25)
#define bWUU0_WUPE26      (sWUU0->PE2.BITS.WUPE26)
#define bWUU0_WUPE27      (sWUU0->PE2.BITS.WUPE27)
#define bWUU0_WUPE28      (sWUU0->PE2.BITS.WUPE28)

/* === Accesos rápidos al registro WUU_ME === */
#define bWUU0_WUME0       (sWUU0->ME.BITS.WUME0)
#define bWUU0_WUME2       (sWUU0->ME.BITS.WUME2)
#define bWUU0_WUME6       (sWUU0->ME.BITS.WUME6)
#define bWUU0_WUME8       (sWUU0->ME.BITS.WUME8)

/* === Accesos rápidos al registro WUU_DE === */
#define bWUU0_WUDE4       (sWUU0->DE.BITS.WUDE4)
#define bWUU0_WUDE6       (sWUU0->DE.BITS.WUDE6)
#define bWUU0_WUDE8       (sWUU0->DE.BITS.WUDE8)

/* === Accesos rápidos al registro WUU_PF === */
#define bWUU0_WUF2        (sWUU0->PF.BITS.WUF2)
#define bWUU0_WUF6        (sWUU0->PF.BITS.WUF6)
#define bWUU0_WUF7        (sWUU0->PF.BITS.WUF7)
#define bWUU0_WUF8        (sWUU0->PF.BITS.WUF8)
#define bWUU0_WUF9        (sWUU0->PF.BITS.WUF9)
#define bWUU0_WUF10       (sWUU0->PF.BITS.WUF10)
#define bWUU0_WUF11       (sWUU0->PF.BITS.WUF11)
#define bWUU0_WUF12       (sWUU0->PF.BITS.WUF12)
#define bWUU0_WUF18       (sWUU0->PF.BITS.WUF18)
#define bWUU0_WUF19       (sWUU0->PF.BITS.WUF19)
#define bWUU0_WUF20       (sWUU0->PF.BITS.WUF20)
#define bWUU0_WUF22       (sWUU0->PF.BITS.WUF22)
#define bWUU0_WUF23       (sWUU0->PF.BITS.WUF23)
#define bWUU0_WUF24       (sWUU0->PF.BITS.WUF24)
#define bWUU0_WUF25       (sWUU0->PF.BITS.WUF25)
#define bWUU0_WUF26       (sWUU0->PF.BITS.WUF26)
#define bWUU0_WUF27       (sWUU0->PF.BITS.WUF27)
#define bWUU0_WUF28       (sWUU0->PF.BITS.WUF28)

/* === Accesos rápidos al registro WUU_FILT === */
#define bWUU0_FILTE1      (sWUU0->FILT.BITS.FILTE1)
#define bWUU0_FILTE2      (sWUU0->FILT.BITS.FILTE2)
#define bWUU0_FILTF1      (sWUU0->FILT.BITS.FILTF1)
#define bWUU0_FILTF2      (sWUU0->FILT.BITS.FILTF2)

/* === Accesos rápidos al registro WUU_PDC1 === */
#define bWUU0_WUPDC2      (sWUU0->PDC1.BITS.WUPDC2)
#define bWUU0_WUPDC6      (sWUU0->PDC1.BITS.WUPDC6)
#define bWUU0_WUPDC7      (sWUU0->PDC1.BITS.WUPDC7)
#define bWUU0_WUPDC8      (sWUU0->PDC1.BITS.WUPDC8)
#define bWUU0_WUPDC9      (sWUU0->PDC1.BITS.WUPDC9)
#define bWUU0_WUPDC10     (sWUU0->PDC1.BITS.WUPDC10)
#define bWUU0_WUPDC11     (sWUU0->PDC1.BITS.WUPDC11)
#define bWUU0_WUPDC12     (sWUU0->PDC1.BITS.WUPDC12)

/* === Accesos rápidos al registro WUU_PDC2 === */
#define bWUU0_WUPDC18     (sWUU0->PDC2.BITS.WUPDC18)
#define bWUU0_WUPDC19     (sWUU0->PDC2.BITS.WUPDC19)
#define bWUU0_WUPDC20     (sWUU0->PDC2.BITS.WUPDC20)
#define bWUU0_WUPDC22     (sWUU0->PDC2.BITS.WUPDC22)
#define bWUU0_WUPDC23     (sWUU0->PDC2.BITS.WUPDC23)
#define bWUU0_WUPDC24     (sWUU0->PDC2.BITS.WUPDC24)
#define bWUU0_WUPDC25     (sWUU0->PDC2.BITS.WUPDC25)
#define bWUU0_WUPDC26     (sWUU0->PDC2.BITS.WUPDC26)
#define bWUU0_WUPDC27     (sWUU0->PDC2.BITS.WUPDC27)
#define bWUU0_WUPDC28     (sWUU0->PDC2.BITS.WUPDC28)
#define bWUU0_WUPDC29     (sWUU0->PDC2.BITS.WUPDC29)
#define bWUU0_WUPDC31     (sWUU0->PDC2.BITS.WUPDC31)

/* === Accesos rápidos al registro WUU_FDC === */
#define bWUU0_FILTC1      (sWUU0->FDC.BITS.FILTC1)
#define bWUU0_FILTC2      (sWUU0->FDC.BITS.FILTC2)

/* === Accesos rápidos al registro WUU_PMC === */
#define bWUU0_WUPMC2      (sWUU0->PMC.BITS.WUPMC2)
#define bWUU0_WUPMC6      (sWUU0->PMC.BITS.WUPMC6)
#define bWUU0_WUPMC7      (sWUU0->PMC.BITS.WUPMC7)
#define bWUU0_WUPMC8      (sWUU0->PMC.BITS.WUPMC8)
#define bWUU0_WUPMC9      (sWUU0->PMC.BITS.WUPMC9)
#define bWUU0_WUPMC10     (sWUU0->PMC.BITS.WUPMC10)
#define bWUU0_WUPMC11     (sWUU0->PMC.BITS.WUPMC11)
#define bWUU0_WUPMC12     (sWUU0->PMC.BITS.WUPMC12)
#define bWUU0_WUPMC18     (sWUU0->PMC.BITS.WUPMC18)
#define bWUU0_WUPMC19     (sWUU0->PMC.BITS.WUPMC19)
#define bWUU0_WUPMC20     (sWUU0->PMC.BITS.WUPMC20)
#define bWUU0_WUPMC22     (sWUU0->PMC.BITS.WUPMC22)
#define bWUU0_WUPMC23     (sWUU0->PMC.BITS.WUPMC23)
#define bWUU0_WUPMC24     (sWUU0->PMC.BITS.WUPMC24)
#define bWUU0_WUPMC25     (sWUU0->PMC.BITS.WUPMC25)
#define bWUU0_WUPMC26     (sWUU0->PMC.BITS.WUPMC26)
#define bWUU0_WUPMC27     (sWUU0->PMC.BITS.WUPMC27)
#define bWUU0_WUPMC28     (sWUU0->PMC.BITS.WUPMC28)
#define bWUU0_WUPMC29     (sWUU0->PMC.BITS.WUPMC29)
#define bWUU0_WUPMC31     (sWUU0->PMC.BITS.WUPMC31)

/* === Accesos rápidos al registro WUU_FMC === */
#define bWUU0_FILTM1      (sWUU0->FMC.BITS.FILTM1)
#define bWUU0_FILTM2      (sWUU0->FMC.BITS.FILTM2)




#endif /* MCX153_WUU_H */