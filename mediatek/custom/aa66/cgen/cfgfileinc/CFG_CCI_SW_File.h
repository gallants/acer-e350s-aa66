#ifndef _CFG_CCI_SW_FILE_H
#define _CFG_CCI_SW_FILE_H

typedef struct
{
	char CCI_SW[64];
	int need_to_write;
}File_CCI_SW_Struct;

#define CFG_FILE_CCI_SW_REC_SIZE    sizeof(File_CCI_SW_Struct)
#define CFG_FILE_CCI_SW_REC_TOTAL   1

#endif
