#ifndef _CFG_ACER_NUMBER_FILE_H
#define _CFG_ACER_NUMBER_FILE_H

typedef struct
{
	char ACER_NUMBER[64];
}File_ACER_NUMBER_Struct;

#define CFG_FILE_ACER_NUMBER_REC_SIZE    sizeof(File_ACER_NUMBER_Struct)
#define CFG_FILE_ACER_NUMBER_REC_TOTAL   1

#endif
