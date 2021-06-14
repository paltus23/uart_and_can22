/*
 * buffs.h
 *
 *  Created on: Jun 13, 2021
 *      Author: maksim
 */

#ifndef INC_BUFFS_H_
#define INC_BUFFS_H_

#include "common.h"

typedef struct circ_buff_t{
	u8* p_buff;
	u32 cap;
	u32 pos_read;
	u32 pos_write;
	u8 fl_overflow;

}circ_buff_t;

#define circ_buff_clear(buff)  {buff.pos_read=0; buff.pos_write=0; buff.fl_overflow=0;}
i32 circ_buff_add(circ_buff_t* buff, u8* data, u32 len);
i32 circ_buff_get(circ_buff_t* buff, u8* data, u32 len);
u32 circ_buff_len(circ_buff_t* buff);

typedef struct buff_t{
	u8* p_buff;
	u32 cap;
	u32 len;
}buff_t;

#define buff_clear(buff)  {buff.len=0;}
u8 buff_write(buff_t* buff, u8* data, u32 len);
u8 buff_add(buff_t* buff, u8* data, u32 len);

#endif /* INC_BUFFS_H_ */
