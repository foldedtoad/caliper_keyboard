/*
 *   app_uicr.h
 *
 * Copyright (c) 2023, Callender-Consulting LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */ 
#ifndef __APP_UICR_H
#define __APP_UICR_H

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

typedef enum{
    INVALID_LINE_END = 0,
    ASCIIZ           = 1,  // ASCII string with zero end
    NEWLINE          = 2,  // ASCII string with \n and zero end
} line_end_t;

typedef enum{
    INVALID_STANDARD = 0,
    INCLUDE          = 1,  // include standard unit: mm or inch literal
    EXCLUDE          = 2,  // no standard unit literal
} standard_t;

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void       app_uicr_init(void);
line_end_t app_uicr_get_line_end(void);
void       app_uicr_set_line_end(line_end_t line_end);
standard_t app_uicr_get_standard(void);
void       app_uicr_set_standard(standard_t standard);

#endif  /* __APP_UICR_H */