#pragma once

/* blocking but frser-aware variant */
uint8_t getline(unsigned char *buf, unsigned char len);

/* Non-blocking but not frser-aware variant */
uint8_t getline_mc(unsigned char *buf, unsigned char len);

void sendstr_P(PGM_P str);
void sendstr(const unsigned char * str);
unsigned char* scanfor_notspace(unsigned char *buf);
unsigned char* scanfor_space(unsigned char *buf);
uint8_t tokenize(unsigned char *rcvbuf,unsigned char** ptrs);
void sendcrlf(void);
void luint2outdual(unsigned long int val);
void u32outdec(unsigned long int val);
