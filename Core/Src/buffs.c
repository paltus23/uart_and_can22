#include "buffs.h"

//TODO return size of wrote bytes
inline i32 circ_buff_add(circ_buff_t* buff, u8* data, u32 len)  {

	if ( buff->pos_write<buff->pos_read) {

		memcpy(buff->p_buff+buff->pos_write, data, min(buff->pos_read - buff->pos_write,len));
		buff->pos_write += min(buff->pos_read - buff->pos_write,len);
	}
	else{
		if((buff->pos_write-buff->pos_read) == 0 && buff->fl_overflow == 1)
			return 0;
		if ( (buff->pos_write+len)<buff->cap) {
			memcpy(buff->p_buff+buff->pos_write, data, len);
			buff->pos_write+=len;
		}
		else {
			u32 len1 = buff->cap - buff->pos_write;
			memcpy(buff->p_buff+buff->pos_write, data, len1);
			memcpy(buff->p_buff, data+len1, min(buff->pos_read, len-len1));

			buff->pos_write = min(buff->pos_read,len-len1);
			buff->fl_overflow=1;
		}
	}

	return 1;

}
inline i32 circ_buff_get(circ_buff_t* buff, u8* data, u32 len)  {
	if(len > buff->cap)
		return 0;



	if ( buff->pos_write>buff->pos_read) {
		len = min(len, buff->pos_write - buff->pos_read);

		memcpy(data, buff->p_buff+buff->pos_read, len);
		buff->pos_read += len;
		return len;
	}
	else{

		if((buff->pos_write-buff->pos_read) == 0 && buff->fl_overflow == 0)
			return 0;

		if((buff->pos_read + len) < buff->cap){
			memcpy(data, buff->p_buff+buff->pos_read, len);
			buff->pos_read += len;
			return len;

		}
		else{
			u32 len1 = buff->cap - buff->pos_read;
			memcpy(data, buff->p_buff+buff->pos_read, len1);
			memcpy(data+len1, buff->p_buff, min(buff->pos_write, len-len1));

			buff->fl_overflow=0;
			buff->pos_read = min(buff->pos_write, len-len1);
			return len1+min(buff->pos_write, len-len1);
		}

	}



}
inline u32 circ_buff_len(circ_buff_t* buff) {
	if ( buff->pos_write>buff->pos_read) {
		u32 len =  buff->pos_write - buff->pos_read;

		return len;
	}
	else{

		if((buff->pos_write-buff->pos_read) == 0 && buff->fl_overflow == 1)
			return buff->cap;

		u32 len = buff->pos_read-buff->cap;
		len += buff->pos_write;
		return len;
	}

}
//u8 circ_buff_add_with_overflow(circ_buff_t* buff, u8* data, u32 len)  {
//	if ( (buff->len+len)<=buff->cap) {
//		memcpy(buff->p_buff+buff->len, data, len); buff->len+=len;	return 1;
//	}
//	else return 0;
//}




inline u8 buff_write(buff_t* buff, u8* data, u32 len)  {
	if (len<=buff->cap) {		memcpy(buff->p_buff, data, len); buff->len=len;	return 1;}
	else return 0;
}
inline u8 buff_add(buff_t* buff, u8* data, u32 len)  {
	if ( (buff->len+len)<=buff->cap) {		memcpy(buff->p_buff+buff->len, data, len); buff->len+=len;	return 1;}
	else return 0;
}
