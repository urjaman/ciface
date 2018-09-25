#pragma once

/* blocking interface */
void ciface_main(void);

/* non-blocking interface, call init once and run in a loop */
void ciface_init(void);
void ciface_run(void);


#define MAXTOKENS 16
#define RECVBUFLEN 64

struct ciface_info {
	unsigned char* tptrs[MAXTOKENS];
	unsigned char rbuf[RECVBUFLEN];
	unsigned char tok_cnt;
	unsigned char gl_i;
};

void ciface_yield(void);

extern struct ciface_info ciface_mi;

#ifndef MULTI_CIFACE
#define CIFACE_INFO(x) ciface_mi.x
#else
#define CIFACE_INFO(x) ciface_ip->x
extern struct ciface_info *ciface_ip;
#endif

#define tokenptrs CIFACE_INFO(tptrs)
#define token_count CIFACE_INFO(tok_cnt)
#define ciface_recvbuf CIFACE_INFO(rbuf)
#define getline_i CIFACE_INFO(gl_i)

#ifndef CIFACE_USE_NEW_UART_FN
#define ciface_recv RECEIVE
#define ciface_send SEND
#ifdef PEEK
#define ciface_peek PEEK
#endif
#define ciface_isdata uart_isdata
#endif
