/*
 * Mem.h
 *
 *  Created on: Feb 13, 2017
 *      Author: fernando
 */

#ifndef MEM_H_
#define MEM_H_

#ifdef __cplusplus
extern "C" {
#endif

int getMemInfo(unsigned int *addr, unsigned int *size);
int Allocate_RAM(void **mem_map, void **ram_addr, unsigned int * addr, const int buffer_size);
int Release_RAM(void *mem_map, const int buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* MEM_H_ */
