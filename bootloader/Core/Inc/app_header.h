/*
 * app_header.h
 *
 *  Created on: Oct 2, 2026
 *      Author: sumed
 */

#ifndef INC_APP_HEADER_H_
#define INC_APP_HEADER_H_

typedef struct
{
    uint32_t magic;
    uint32_t size;
    uint32_t crc;
    uint32_t version;
} app_header_t;


#endif /* INC_APP_HEADER_H_ */
