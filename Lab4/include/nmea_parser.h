/**
 * @file nmea_parser.h
 * @brief Parser de sentencias NMEA para extraer información GPS.
 */

#ifndef NMEA_PARSER_H
#define NMEA_PARSER_H

#include <stdbool.h>

/**
 * @struct gps_data_t
 * @brief Estructura que contiene los datos extraídos de una sentencia NMEA.
 */
typedef struct {
    bool valid_fix;     /**< Indica si hay una solución de posición válida. */
    double latitude;    /**< Latitud en grados decimales. */
    double longitude;   /**< Longitud en grados decimales. */
    char lat_dir;       /**< Dirección de la latitud ('N' o 'S'). */
    char lon_dir;       /**< Dirección de la longitud ('E' o 'W'). */
    int hour;           /**< Hora (UTC). */
    int minute;         /**< Minuto (UTC). */
    int second;         /**< Segundo (UTC). */
    int day;            /**< Día del mes. */
    int month;          /**< Mes. */
    int year;           /**< Año (completo, e.g., 2025). */
    int satellites;     /**< Número de satélites en uso. */
    double altitude;    /**< Altitud sobre el nivel del mar (en metros). */
    int fix_quality;    /**< Calidad de la solución (0: no fix, 1: GPS fix, 2: DGPS fix, etc). */
} gps_data_t;

/**
 * @brief Parsea una línea NMEA y extrae los datos GPS relevantes.
 * 
 * @param line Cadena con la sentencia NMEA (por ejemplo, GPGGA, GPRMC).
 * @param data Puntero a una estructura gps_data_t donde se almacenarán los datos extraídos.
 * @return true si el parsing fue exitoso y se obtuvo una solución válida, false en caso contrario.
 */
bool nmea_parse_line(const char *line, gps_data_t *data);

#endif // NMEA_PARSER_H
