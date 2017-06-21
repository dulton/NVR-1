#ifndef _G726_H_
#define	_G726_H_

void g726_Init();

void g726_Encode(unsigned char *speech,char *bitstream);
void g726_Decode(int index,unsigned char *bitstream,unsigned char *speech,int len);

#endif

