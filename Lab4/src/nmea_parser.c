/**
 * @file nmea_parser.c
 * @brief Implementación del parser para sentencias NMEA estándar (GPRMC, GPGGA, GNGLL).
 */

#include "include/nmea_parser.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * @brief Convierte una coordenada NMEA en formato ddmm.mmmm a grados decimales.
 *
 * @param nmea_coord Cadena que representa la coordenada en formato NMEA.
 * @return Coordenada convertida en grados decimales.
 */
static double nmea_to_decimal(const char *nmea_coord) {
    double deg = atof(nmea_coord);
    int degrees = (int)(deg / 100);
    double minutes = deg - (degrees * 100);
    return degrees + (minutes / 60.0);
}

/**
 * @brief Parsea una línea NMEA (GPRMC, GPGGA o GNGLL) y extrae los datos relevantes.
 *
 * @param line Línea de texto que contiene la sentencia NMEA, iniciando con '$'.
 * @param data Puntero a estructura `gps_data_t` donde se almacenarán los resultados.
 * @return true si se extrajeron datos válidos con una solución GPS, false si no hay solución o la línea no es soportada.
 */
bool nmea_parse_line(const char *line, gps_data_t *data) {
    if (!line || line[0] != '$') return false;

    char fields[20][20] = {0};
    int field_count = 0;
    const char *p = line;

    // Separar los campos por comas
    while (*p && field_count < 20) {
        int i = 0;
        while (*p && *p != ',' && *p != '*') {
            fields[field_count][i++] = *p++;
        }
        fields[field_count][i] = '\0';
        if (*p == ',' || *p == '*') p++;
        field_count++;
    }

    // Procesar sentencia GPRMC
    if (strncmp(line, "$GPRMC", 6) == 0) {
        if (fields[2][0] != 'A') {
            data->valid_fix = false;
            return false;
        }

        data->valid_fix = true;
        sscanf(fields[1], "%2d%2d%2d", &data->hour, &data->minute, &data->second);
        data->latitude = nmea_to_decimal(fields[3]);
        data->lat_dir = fields[4][0];
        if (data->lat_dir == 'S') data->latitude *= -1;

        data->longitude = nmea_to_decimal(fields[5]);
        data->lon_dir = fields[6][0];
        if (data->lon_dir == 'W') data->longitude *= -1;

        sscanf(fields[9], "%2d%2d%2d", &data->day, &data->month, &data->year);
        data->year += 2000;

        return true;
    }

    // Procesar sentencia GPGGA
    else if (strncmp(line, "$GPGGA", 6) == 0) {
        data->latitude = nmea_to_decimal(fields[2]);
        data->lat_dir = fields[3][0];
        if (data->lat_dir == 'S') data->latitude *= -1;

        data->longitude = nmea_to_decimal(fields[4]);
        data->lon_dir = fields[5][0];
        if (data->lon_dir == 'W') data->longitude *= -1;

        data->fix_quality = atoi(fields[6]);
        data->satellites = atoi(fields[7]);
        data->altitude = atof(fields[9]);
        data->valid_fix = data->fix_quality > 0;

        return true;
    }

    // Procesar sentencia GNGLL
    else if (strncmp(line, "$GNGLL", 6) == 0) {
        if (fields[6][0] != 'A') {
            data->valid_fix = false;
            return false;
        }

        data->latitude = nmea_to_decimal(fields[1]);
        data->lat_dir = fields[2][0];
        if (data->lat_dir == 'S') data->latitude *= -1;

        data->longitude = nmea_to_decimal(fields[3]);
        data->lon_dir = fields[4][0];
        if (data->lon_dir == 'W') data->longitude *= -1;

        sscanf(fields[5], "%2d%2d%2d", &data->hour, &data->minute, &data->second);
        data->valid_fix = true;

        // Campos no presentes en GNGLL
        data->day = data->month = data->year = 0;
        data->satellites = 0;
        data->altitude = 0.0;
        data->fix_quality = 1;

        return true;
    }

    return false;
}
