#ifndef _ONENET_H_
#define _ONENET_H_

#include "iap.h"

void OneNet_SendData_Get(char n);

char OneNet_RevPro_HTTP(unsigned char *dataPtr);

//PTR_ipd  CharToInt( char * dataPtr,ptr_struct *Ptr_Struct,char n);
char Get_Splice_Points(unsigned char *dataPtr,ptr_struct * PTRSTRUCT);
char Merge_Pkg( char * dataPtr,char * databuff, ptr_struct *Ptr_Struct,char n);
char GetPkgData_andCheck(char * data_buf,PKG_Data* PKG_DATA);
char GetXmlData_andCheck(char * data_buf,XML_Data* XML_DATA);
unsigned short bitop(unsigned short a, unsigned int k, unsigned int b);
#endif
