#include <app.h>
#include <debug.h>
#include <arch/arm.h>
#include <dev/udc.h>
#include <string.h>
#include <kernel/thread.h>
#include <kernel/event.h>
#include <arch/ops.h>
#include <target.h>
#include <platform.h>
#include <platform/mt_reg_base.h>
#include <platform/boot_mode.h>
#include <platform/mtk_wdt.h>
#include <platform/mt_rtc.h>
#include <platform/bootimg.h>
#include <mt_partition.h>
#include <platform/mtk_nand.h>

/*For image write*/
#include "sparse_format.h"
#include "download_commands.h"
#include "sparse_state_machine.h"
#include <platform/mmc_core.h>
#include <platform/mt_gpt.h>

#include "fastboot.h"


#define MODULE_NAME "FASTBOOT_DOWNLOAD"
#define MAX_RSP_SIZE 64


extern void *download_base;
extern unsigned download_max;
extern unsigned download_size;
extern unsigned fastboot_state;

/*LXO: !Download related command*/

#define ROUND_TO_PAGE(x,y) (((x) + (y)) & (~(y)))
#define INVALID_PTN -1
/*#define DBG_LV SPEW*/
#define DBG_LV 0
#define DBG_DCACHE 2
//For test: Display info on boot screen
#define DISPLAY_INFO_ON_LCM


unsigned start_time_ms;
#define TIME_STAMP gpt4_tick2time_ms(gpt4_get_current_tick())
#define TIME_START {start_time_ms = gpt4_tick2time_ms(gpt4_get_current_tick());}
#define TIME_ELAPSE (gpt4_tick2time_ms(gpt4_get_current_tick()) - start_time_ms)

extern int usb_write(void *buf, unsigned len);
extern int usb_read(void *buf, unsigned len);
static void init_display_xy();
static void display_info(const char* msg);
static void display_progress(const char* msg_prefix, unsigned size, unsigned totle_size);
static void display_speed_info(const char* msg_prefix, unsigned size);
static void fastboot_fail_wrapper(const char* msg);
static void fastboot_ok_wrapper(const char* msg, unsigned size);
static unsigned hex2unsigned(const char *x);



#define BOOT_IMAGE_SIZE 	18*1024*1024  //assume that is 18M, now is about 5M.
#define CACHE_PADDING_SIZE 	BLK_SIZE

u32 IMAGE_TRUNK_SIZE  = (8*1024*1024);    //8M default
void set_image_trunk_size(u32 size)
{
	IMAGE_TRUNK_SIZE = size;
}

#define CACHE_PAGE_SIZE 		(IMAGE_TRUNK_SIZE)
#define DCACHE_SIZE 			(2*CACHE_PAGE_SIZE+2*CACHE_PADDING_SIZE)
#define MEMORY_SIZE_REQ 		(DCACHE_SIZE>BOOT_IMAGE_SIZE ? DCACHE_SIZE+BOOT_IMAGE_SIZE : 2*BOOT_IMAGE_SIZE)
#define BOOT_IMG_ADDR_OFFSET 	(DCACHE_SIZE>BOOT_IMAGE_SIZE ? DCACHE_SIZE : BOOT_IMAGE_SIZE)
/*dual cache pattern:     | PADDING1 | CACHE1 |PADDING2 | CACHE2 |+ boot image temporal */

#define SIGNAL_RESCHEDULE 0  //true; active. 0 passive.

#if defined(MTK_EMMC_SUPPORT)  //use another macro.
#define EMMC_TYPE
#endif

typedef enum {
	S_CHECK_ONLY = 0,
	S_CHECK_AND_DOWNLOAD,
} SECURITY_STEP_E;

typedef struct cache
{
	u8* padding_buf;
	u8* cache_buf;
	u32 padding_length;  //sparse image boundary problem.
	u32 content_length;  //if this is 0, indicate this the last package.
	event_t content_available;
	event_t cache_available;
	u32 check_sum;   //reserved.
}cache_t;

typedef struct boot_image_info
{
	u8* boot_image_address;
	BOOL is_boot_image;
	u32 offset;
}boot_image_info_t;

typedef struct engine_context
{
	cache_t  dual_cache[2];
	u32 flipIdxR;     //receive buffer shift
	u32 flipIdxW;    //write buffer shift
	event_t thr_end_ev; // recieve thread exit sync.
	u32 b_error ; // something is wrong, should exit.
	SECURITY_STEP_E security_step;  //security download need 2 step.
	BOOL security_enabled;
	boot_image_info_t boot_info;
}engine_context_t;

static engine_context_t ctx;

void init_engine_context(engine_context_t* tx)
{
	u8* cache_base = (u8*)download_base;  // use absolute address.

	tx->dual_cache[0].padding_buf = cache_base;
	tx->dual_cache[0].cache_buf = cache_base+CACHE_PADDING_SIZE;
	tx->dual_cache[1].padding_buf = cache_base+CACHE_PADDING_SIZE+CACHE_PAGE_SIZE;
	tx->dual_cache[1].cache_buf = cache_base+CACHE_PADDING_SIZE+CACHE_PAGE_SIZE+CACHE_PADDING_SIZE;
	tx->boot_info.boot_image_address = cache_base+BOOT_IMG_ADDR_OFFSET;

	tx->dual_cache[0].padding_length = tx->dual_cache[1].padding_length = 0;

	event_init(&tx->dual_cache[0].content_available, 0, EVENT_FLAG_AUTOUNSIGNAL);//no data in cache
	event_init(&tx->dual_cache[1].content_available, 0, EVENT_FLAG_AUTOUNSIGNAL);//no data in cache

	event_init(&tx->dual_cache[0].cache_available, 1, EVENT_FLAG_AUTOUNSIGNAL);    //can receive from usb
	event_init(&tx->dual_cache[1].cache_available, 1, EVENT_FLAG_AUTOUNSIGNAL);    //can receive from usb

	event_init(&tx->thr_end_ev, 0, EVENT_FLAG_AUTOUNSIGNAL);//do not end.
	tx->b_error = 0;
	tx->flipIdxR = tx->flipIdxW = 0;
	tx->security_step = S_CHECK_ONLY;
	tx->security_enabled = FALSE;
	tx->boot_info.is_boot_image = FALSE;
	tx->boot_info.offset = 0;

}

inline u32 cache_shift(u32 pre)
{
	return pre ^ 0x01;
}



typedef struct storage_info
{
#if defined(EMMC_TYPE)
	int is_sparse_image;
	unsparse_status_t unsparse_status;
#endif
	int first_run;
	u64 image_base_addr;
	u64 bulk_image_offset;
	u32 to_write_data_len;
	u8 partition_name[32];
	BOOL checksum_enabled;
}storage_info_t;

static storage_info_t sto_info;
void init_sto_info(storage_info_t* s, BOOL checksum_enabled)
{
#if defined(EMMC_TYPE)
	s->is_sparse_image = 0;
	s->first_run = 1;
	mmc_init_unsparse_status(&(s->unsparse_status), 0, checksum_enabled);
#endif
 	s->image_base_addr = 0;
	s->bulk_image_offset = 0;
	s->to_write_data_len = 0;
	memset(s->partition_name, 0, 32);
	s->checksum_enabled = checksum_enabled;
}

void abort_engine(engine_context_t* tx)
{
	tx->b_error = 1;

	event_signal(&tx->dual_cache[0].cache_available, SIGNAL_RESCHEDULE);
	event_signal(&tx->dual_cache[1].cache_available, SIGNAL_RESCHEDULE);
	event_signal(&tx->dual_cache[0].content_available, SIGNAL_RESCHEDULE);
	event_signal(&tx->dual_cache[1].content_available, SIGNAL_RESCHEDULE);
	event_signal(&tx->thr_end_ev, SIGNAL_RESCHEDULE);
}

void destroy_engine(engine_context_t* tx)
{
	event_destroy(&tx->dual_cache[0].cache_available);
	event_destroy(&tx->dual_cache[1].cache_available);
	event_destroy(&tx->dual_cache[0].content_available);
	event_destroy(&tx->dual_cache[1].content_available);
	event_destroy(&tx->thr_end_ev);
}

BOOL is_sparse_image(u8* data, u32 length)
{
	sparse_header_t *sparse_header;
	u32* magic_number = (u32*) data;

	sparse_header = (sparse_header_t *) data;

	dprintf(DBG_LV, "is_sparse_image(), data:0x%x,, n sparse_header->magic = 0x%x\n",*(int*)data, sparse_header->magic );

	return (sparse_header->magic == SPARSE_HEADER_MAGIC) ? TRUE : FALSE;
}

//interface
int get_partition_name(u8* data, u32 length, u8* buf)
{
	return get_mountpoint_from_image(data, length, buf);
}

#if defined(EMMC_TYPE)
BOOL write_to_emmc(u8* data, u32 length)
{
	u64 paritition_size = 0;
	u64 size_wrote = 0;
	int next_flip = 0;
	u32 index;
	u32 pre_chksum = 0;
	u32 post_chksum = 0;
	int r;

	if(sto_info.first_run)
	{
		r = get_partition_name(data, length, sto_info.partition_name);
		if(r < 0)
		{
			display_info("\nASSERT!! get_partition_name() Fail");
			return FALSE;
		}
		if(!strncmp(sto_info.partition_name, "boot", 8))
		{
			ctx.boot_info.is_boot_image = TRUE;
			ctx.boot_info.offset = 0;
		}
		index = partition_get_index(sto_info.partition_name);
		if(index == -1)
		{
			display_info("\nASSERT!! Brick phone??");
			return FALSE;
		}

		if(!is_support_flash(index))
		{
			display_info(sto_info.partition_name);
			display_info("\nASSERT!! Dont support system??");
			return FALSE;
		}

		paritition_size = partition_get_size(index);
		dprintf(DBG_LV, "[index:%d]-[partitionSize:%lld]-[downSize:%lld]\n", index, paritition_size, sto_info.to_write_data_len);

		if (ROUND_TO_PAGE(sto_info.to_write_data_len,511) > paritition_size)
		{
			display_info("\nsize too large, space small.");
			dprintf(DBG_LV, "size too large, space small.");
			return FALSE;
		}

		sto_info.image_base_addr = partition_get_offset(index);
		sto_info.unsparse_status.image_base_addr = sto_info.image_base_addr;
		sto_info.is_sparse_image = is_sparse_image(data, length);
		sto_info.first_run = 0;
	}

	//boot image do not need write to image at this function. it is in flash function.
	if(ctx.boot_info.is_boot_image)
	{
		dprintf(DBG_LV, "boot img: len: %d\n", length);
		dprintf(DBG_LV, "data: %08X\n", data);
		dprintf(DBG_LV, "ctx.boot_info.boot_image_address: %08X, ctx.boot_info.offset %u, \n", ctx.boot_info.boot_image_address , ctx.boot_info.offset);

		memcpy(ctx.boot_info.boot_image_address + ctx.boot_info.offset, data, length);
		ctx.boot_info.offset += length;
		return TRUE;
	}

	if(sto_info.is_sparse_image)
	{
		next_flip = cache_shift(ctx.flipIdxR);

		sto_info.unsparse_status.buf = data;
		sto_info.unsparse_status.size = length;
		mmc_write_sparse_data(&sto_info.unsparse_status);

		if(sto_info.unsparse_status.handle_status == S_DA_SDMMC_SPARSE_INCOMPLETE)
		{
			ctx.dual_cache[next_flip].padding_length = sto_info.unsparse_status.size;
			memcpy(ctx.dual_cache[next_flip].padding_buf +(CACHE_PADDING_SIZE-sto_info.unsparse_status.size)
				, sto_info.unsparse_status.buf
				, sto_info.unsparse_status.size);
		}
		else if (sto_info.unsparse_status.handle_status== S_DONE)
		{
			ctx.dual_cache[next_flip].padding_length = 0;
		}
		else
		{
			//some error
			dprintf(DBG_LV, "write_to_emmc() Failed. handle_status(%d)\n", sto_info.unsparse_status.handle_status);
			display_info("\nError in write sparse image in EMMC.");
			return FALSE;
		}
	}
	else
	{
		size_wrote = emmc_write(sto_info.image_base_addr+sto_info.bulk_image_offset , (void*)data, length);
		if (size_wrote  != length)
		{
			dprintf(DBG_LV, "write_to_emmc() Failed. act(%lld) != want(%lld)\n", size_wrote, length);
			display_info("\nError in write bulk in EMMC.");
			return FALSE;
		}
		if(sto_info.checksum_enabled)
		{
			pre_chksum = calc_checksum(data, (u32)length);
			if(length != emmc_read(sto_info.image_base_addr+sto_info.bulk_image_offset, data,  length))
			{
				dprintf(DBG_LV, "emmc_read() Failed.\n");
				display_info("\nError in Read bulk EMMC.");
				return FALSE;
			}

			post_chksum = calc_checksum(data, (u32)length);

			if(post_chksum != pre_chksum)
			{
				dprintf(DBG_LV, "write_to_emmc() Failed. checksum error\n");
				display_info("\nWrite bulk in EMMC. Checksum Error");
				return FALSE;
			}
		}

		sto_info.bulk_image_offset += size_wrote;
	}
	return TRUE;
}

#else
BOOL write_to_nand(u8* data, u32 length)
{
	u64 paritition_size = 0;
	int next_flip = 0;
	u32 index;
	BOOL partition_type;
	s8* p_type;
	//u32 pre_chksum = 0;
	//u32 post_chksum = 0;
	int r;

	if(sto_info.first_run)
	{
		r = get_partition_name(data, length, sto_info.partition_name);
		if(r < 0)
		{
			display_info("ASSERT!! get_partition_name() Fail");
			return FALSE;
		}

		index = partition_get_index(sto_info.partition_name);  //
		if(index == -1)
		{
			display_info("ASSERT!! Brick phone??");
			return FALSE;
		}

		if(!is_support_flash(index))
		{
			display_info("ASSERT!! Dont support system??");
			return FALSE;
		}
		//verify boot partition.
		if (!strcmp(sto_info.partition_name, "boot") || !strcmp(sto_info.partition_name, "recovery"))
		{
			if (memcmp((void *)data, BOOT_MAGIC, strlen(BOOT_MAGIC)))
			{
				display_info("image is not a boot image");
				return FALSE;
			}
		}

		paritition_size = partition_get_size(index);
		dprintf(DBG_LV, "[index:%d]-[partitionSize:%lld]-[downSize:%d]\n", index, paritition_size, sto_info.to_write_data_len);

		if (ROUND_TO_PAGE(sto_info.to_write_data_len,511) > paritition_size)
		{
			display_info("size too large, space small.");
			dprintf(DBG_LV, "size too large, space small.");
			return FALSE;
		}

		partition_get_type(index,&p_type);
		partition_type = (!strcmp(p_type,"yaffs2")) ? TRUE : FALSE;

		sto_info.image_base_addr = partition_get_offset(index);
		//NAND has no sparse image.
		//sto_info.unsparse_status.image_base_addr = sto_info.image_base_addr;
		//sto_info.is_sparse_image = is_sparse_image(data, length);
		sto_info.first_run = 0;
	}

	if (0 != nand_write_img((u32)(sto_info.image_base_addr+sto_info.bulk_image_offset), (void*)data, length,(u32)paritition_size, partition_type))
	{
		dprintf(DBG_LV, "nand_write_img() Failed.\n");
		display_info("Error in write bulk in NAND.");
		return FALSE;
	}
	if(sto_info.checksum_enabled)
	{
		//NAND do not support read() now.
	}

	sto_info.bulk_image_offset += length;

	return TRUE;
}

#endif
inline BOOL write_data(u8* data, u32 length)
{
	if(data == 0 ||length==0)
	{
		return TRUE;
	}
#if defined(EMMC_TYPE)
	return write_to_emmc(data, length);
#else
	//NAND is not support temporally
	//return write_to_nand(data, length);
	return FALSE;
#endif

}

inline void enable_security(engine_context_t* tx, BOOL en)
{
	tx->security_enabled = en;
}

inline BOOL security_enabled(engine_context_t* tx)
{
	//maybe use macro.
	return tx->security_enabled;
}

inline void set_security_step(engine_context_t* tx, SECURITY_STEP_E s)
{
	tx->security_step = s;
}

//interface
BOOL security_check(/*IN OUT*/u8** pdata, /*IN OUT*/ u32* plength, /*IN*/ u64 image_offset)
{
	return TRUE;
}

int write_storage_proc(void *arg)
{
	u8* data = 0;
	u32 data_len = 0;
	u8 msg[128];
	u64 image_offset = 0;

	dprintf(DBG_DCACHE, "   --[%d] Enter write_storage_proc \n", TIME_STAMP);
	for (;;)
	{
		dprintf(DBG_DCACHE, "   --[%d]Wait ID:%d  cache to read, \n", TIME_STAMP, ctx.flipIdxR);
		event_wait(&(ctx.dual_cache[ctx.flipIdxR].content_available));
		dprintf(DBG_DCACHE, "   --[%d]Obtain ID:%d  cache to read, \n", TIME_STAMP, ctx.flipIdxR);
		if(ctx.b_error)
		{
			sprintf(msg, "\nError USB?\n");
			goto error;
		}
		//if has something to write
		if(ctx.dual_cache[ctx.flipIdxR].content_length !=0  || ctx.dual_cache[ctx.flipIdxR].padding_length !=0)
		{
			data = (u8*)(ctx.dual_cache[ctx.flipIdxR].cache_buf);
			data_len = ctx.dual_cache[ctx.flipIdxR].content_length;

			if(security_enabled(&ctx))
			{
				if(!security_check(&data, &data_len, image_offset))
				{
					//security error.
					sprintf(msg, "\nSecurity deny.\n");
					goto error;
				}
				image_offset += data_len;
			}

			data -=  ctx.dual_cache[ctx.flipIdxR].padding_length;
			data_len += ctx.dual_cache[ctx.flipIdxR].padding_length;
			//if data_len==0, secure img tail met.

			if(!security_enabled(&ctx) || ctx.security_step == S_CHECK_AND_DOWNLOAD)
			{
				dprintf(DBG_DCACHE, "   --[%d]Write ID:%d  to EMMC \n", TIME_STAMP, ctx.flipIdxR);
				if(!write_data(data, data_len))
				{
					//error
					sprintf(msg, "\nWrite data error. \n");
					goto error;
				}
			}
		}

		//last package, should return;
		if (ctx.dual_cache[ctx.flipIdxR].content_length == 0)
		{
			dprintf(DBG_DCACHE, "  --[%d]Write EMMC Fin\n", TIME_STAMP);
			if(security_enabled(&ctx))
			{
				security_check(&data, 0, image_offset); //notify security check that is the end.
			}

			if(ctx.boot_info.is_boot_image)
			{
				//boot image need download_size to flash.
				download_size = sto_info.to_write_data_len;
				//cache using is over, so copy boot image to download_base
				memcpy(download_base, ctx.boot_info.boot_image_address, download_size);
			}

			event_signal(&ctx.dual_cache[0].cache_available, SIGNAL_RESCHEDULE);//prevent from dead lock.
			event_signal(&ctx.dual_cache[1].cache_available, SIGNAL_RESCHEDULE);
			event_signal(&ctx.thr_end_ev, SIGNAL_RESCHEDULE);
			return 0;
		}

		dprintf(DBG_DCACHE, "   --[%d]Notify ID:%d cache writeable\n", TIME_STAMP, ctx.flipIdxR);
		event_signal(&ctx.dual_cache[ctx.flipIdxR].cache_available, SIGNAL_RESCHEDULE); //make this cache writeable again.

		ctx.flipIdxR = cache_shift(ctx.flipIdxR); //change next buffer.
	}
	return 0;
error:
	dprintf(DBG_LV, msg);
	display_info(msg);
	abort_engine(&ctx);
	return (-1);
}



BOOL read_usb_proc(void *arg)
{
	u8 msg[128];
	u32 bytes_already_read = 0;
	u32 data_length = *((u32*)arg);
	u32 bytes_read = 0;
	int bytes_read_once = 0;
	u32 bytes_left = 0;

	dprintf(DBG_DCACHE, "++[%d]Enter read_usb_proc\n", TIME_STAMP);

	while (bytes_already_read  < data_length)
	{
		dprintf(DBG_DCACHE, "++[%d]Wait ID:%d  cache to write\n", TIME_STAMP, ctx.flipIdxW);
		event_wait(&(ctx.dual_cache[ctx.flipIdxW].cache_available));
		dprintf(DBG_DCACHE, "++[%d]Obtain ID:%d  cache to write, \n", TIME_STAMP, ctx.flipIdxW);
		if(ctx.b_error)
		{
			sprintf(msg, "\nError Write?\n");
			goto error;
		}

		bytes_read = 0;
		bytes_left = data_length - bytes_already_read;
		bytes_left = bytes_left >= CACHE_PAGE_SIZE ? CACHE_PAGE_SIZE : bytes_left;

		dprintf(DBG_DCACHE, "++[%d]READ USB to ID:%d\n", TIME_STAMP, ctx.flipIdxW);
		while(bytes_left > 0)
		{
			bytes_read_once = usb_read(ctx.dual_cache[ctx.flipIdxW].cache_buf + bytes_read,  bytes_left);

			if (bytes_read_once < 0)
			{
				abort_engine(&ctx);
				dprintf(DBG_LV, "Read USB error.\n");
				display_info("\nRead USB error\n");
				fastboot_state = STATE_ERROR;
				return FALSE;
			}
			bytes_left -= bytes_read_once;
			bytes_read += bytes_read_once;
		}

		ctx.dual_cache[ctx.flipIdxW].content_length = bytes_read;
		bytes_already_read += bytes_read;

		dprintf(DBG_DCACHE, "++[%d]Notify ID:%d cache readable\n", TIME_STAMP, ctx.flipIdxW);
		event_signal(&ctx.dual_cache[ctx.flipIdxW].content_available, SIGNAL_RESCHEDULE);

		ctx.flipIdxW = cache_shift(ctx.flipIdxW); //change next buffer.

		display_progress("\rTransfer Data", bytes_already_read, data_length);
	}

	if(bytes_already_read  != data_length)
	{
		dprintf(DBG_LV, "ASSERT error.  bytes_already_read  != data_length\n");
		//cause assert.
		*((int*)0x00) = 0;
	}

	dprintf(DBG_DCACHE, "++[%d]USB read Fin\n", TIME_STAMP);
	//last package.
         //must wait for this can write again.
	event_wait(&(ctx.dual_cache[ctx.flipIdxW].cache_available));
	ctx.dual_cache[ctx.flipIdxW].content_length = 0;
	event_signal(&ctx.dual_cache[ctx.flipIdxW].content_available, SIGNAL_RESCHEDULE);

	return TRUE;
error:
	dprintf(DBG_LV, msg);
	display_info(msg);
	abort_engine(&ctx);
	return FALSE;
}

int is_use_ex_download()
{
#if defined(EMMC_TYPE)
		return 1;
#else
		return 0;
#endif
}

int download_ex(u32 data_length)//Big image and parallel transfer.
{
	thread_t *thr;

	init_engine_context(&ctx);
	init_sto_info(&sto_info, FALSE);  //no checksum enabled.
	sto_info.to_write_data_len = data_length;

	enable_security(&ctx, FALSE);
	//Modify the security step according to some variable.
	//set_security_step(&ctx, S_CHECK_ONLY);
	set_security_step(&ctx, S_CHECK_AND_DOWNLOAD); //now is one step to security download.


	thr = thread_create("fastboot", write_storage_proc, 0, DEFAULT_PRIORITY, 16*1024);
	if (!thr)
	{
		return -1;
	}
	thread_resume(thr);

	TIME_START;

	read_usb_proc(&data_length);

	//wait for write thread end.
	event_wait(&ctx.thr_end_ev);

	destroy_engine(&ctx);

	if(ctx.b_error)
	{
		fastboot_fail_wrapper("\n@DOWNLOAD ERROR@");
	}
	else
	{
		fastboot_okay("");
	}
}

int download_standard(u32 data_length)
{
	int r;
	display_info("USB Transferring... ");
	TIME_START;
	r = usb_read(download_base, data_length);

	if ((r < 0) || ((unsigned) r != data_length))
	{
		dprintf(DBG_LV, "Read USB error.\n");
		display_speed_info("Read USB error", data_length);
        		fastboot_fail_wrapper("Read USB error");
		fastboot_state = STATE_ERROR;
		return;
	}
	download_size = data_length;

	dprintf(DBG_LV, "read OK.  data:0x%x\n", *((int*)download_base));
	fastboot_ok_wrapper("USB Transmission OK", data_length);
}

int get_mountpoint_from_image(void *data, unsigned sz, char *mountpoint)
{
	sparse_header_t *sparse_header;
	chunk_header_t *chunk_header;
	unsigned int chunk_data_sz = 0;

	if(sz < 4*1024){
		printf("[get_mountpoint_from_image]sz must be more than one block(4KB)\n");
		return -1;
	}
	sparse_header = (sparse_header_t *) data;
	printf("[get_mountpoint_from_image]data %p, mageic %x\n",data,sparse_header->magic);
	if (sparse_header->magic == SPARSE_HEADER_MAGIC)
	{
			/* Read and skip over sparse image header */
			data += sizeof(sparse_header_t);
			printf("[get_mountpoint_from_image]sparse header size %d, file_hdr_sz: %d\n",sizeof(sparse_header_t), sparse_header->file_hdr_sz);
			if(sparse_header->file_hdr_sz > sizeof(sparse_header_t))
			{
				/* Skip the remaining bytes in a header that is longer than
				* we expected.
				*/
				data += (sparse_header->file_hdr_sz - sizeof(sparse_header_t));
			}


			/* Read and skip over chunk header */
			chunk_header = (chunk_header_t *) data;
			data += sizeof(chunk_header_t);

			if(sparse_header->chunk_hdr_sz > sizeof(chunk_header_t))
			{
				/* Skip the remaining bytes in a header that is longer than
				* we expected.
				*/
				data += (sparse_header->chunk_hdr_sz - sizeof(chunk_header_t));
			}

			chunk_data_sz = sparse_header->blk_sz * chunk_header->chunk_sz;

			if(chunk_header->chunk_type == CHUNK_TYPE_RAW || chunk_data_sz >= 4*1024){
				if(chunk_header->total_sz != (sparse_header->chunk_hdr_sz + chunk_data_sz))
				{
					fastboot_fail_wrapper("Bogus chunk size for chunk type Raw");
					return -1;
				}
				return __check_mountpoint(data,4*1024,mountpoint);
			}else{
				printf("[get_mountpoint_from_image]the first chunk is empty or chunk size %d is littler than 4KB\n",chunk_data_sz);
				return -1;
			}
	}
	else
	{
			return __check_mountpoint(data,4*1024,mountpoint);
	}
}

int __check_mountpoint(void *data, int count, char *mountpoint)
{
	struct ext4_super_block *sb;
	char buf[64];
	if(count < sizeof(struct ext4_super_block)){
		printf("[__check_mountpoint] %d littler than ext4 super block size\n",count);
	  return -1;
	}
	if (0 == memcmp((void *)data, BOOT_MAGIC, strlen(BOOT_MAGIC))){
		strcpy(mountpoint,"boot");
		return 0;
	}
	data += 1024;

	sb = (struct ext4_super_block *)data;

	if(sb->s_magic != EXT4_SUPER_MAGIC){
		printf("[__check_mountpoint]ext4 magic num error, %x data %p\n",sb->s_magic,data);
		return -1;
	}
	memcpy(buf,sb->s_last_mounted,sizeof(sb->s_last_mounted));
	if(strncmp(buf,"/",1)){
		printf("[__check_mountpoint]mountpoint  %s is not support\n",buf);
		return -1;
	}
	printf("[__check_mountpoint]mountpoint is %s\n",buf);
	if(!strcmp(&buf[1],"data")){
			strcpy(mountpoint,"userdata");
	}else{
		memcpy(mountpoint,&buf[1],sizeof(buf)-1);
	}
	return 0;
}


void cmd_download(const char *arg, void *data, unsigned sz)
{
	char response[MAX_RSP_SIZE];
	unsigned len = hex2unsigned(arg);
	u32 available_memory=0;
	int r;

	init_display_xy();
	download_size = 0;
	available_memory = memory_size()-(u32)download_base;

	dprintf(DBG_LV, "Enter cmd_download Data Length:%d, available_memory:%d\n", len, available_memory);

	if (len > download_max)
	{
		dprintf(DBG_LV, "Data is larger than all partitions size in target.\n");
		fastboot_fail_wrapper("Data is larger than all partitions size in target");
		return;
	}

	if(is_use_ex_download())
	{
		if(available_memory < MEMORY_SIZE_REQ)
		{
			dprintf(DBG_LV, "Insufficient memory for DCACHE\n");
			fastboot_fail_wrapper("Insufficient memory for DCACHE");
			return;
		}
	}
	else
	{
		if (len > available_memory)
		{
			dprintf(DBG_LV, "Insufficient memory for whole image\n");
			fastboot_fail_wrapper("Insufficient memory for whole image");
			return;
		}
	}


	snprintf(response, MAX_RSP_SIZE, "DATA%08x", len);
	if (usb_write(response, strlen(response)) < 0)
	{
		return;
	}

	if(is_use_ex_download())
	{
		//use ex download
		download_ex(len);
	}
	else
	{
		//use normal download
		download_standard(len);
	}

	return;
}


/*PROTECTED

void cmd_download(const char *arg, void *data, unsigned sz)
{
	char response[MAX_RSP_SIZE];
	unsigned len = hex2unsigned(arg);
	u32 available_memory=0;
	int r;

	init_display_xy();
	download_size = 0;
	available_memory = memory_size()-(u32)download_base;

	dprintf(DBG_LV, "Enter cmd_download. Length:%d, available_memory:%d\n", len, available_memory);

	if (len > download_max || len > available_memory)
	{
		dprintf(DBG_LV, "data too large.len > download_max.\n");
		fastboot_fail_wrapper("data too large");
		return;
	}

	snprintf(response, MAX_RSP_SIZE, "DATA%08x", len);

	if (usb_write(response, strlen(response)) < 0)
	{
		return;
	}

	display_info("USB Transferring... ");
	TIME_START;
	r = usb_read(download_base, len);

	if ((r < 0) || ((unsigned) r != len))
	{
		dprintf(DBG_LV, "Read USB error.\n");
		display_speed_info("Read USB error", len);
        fastboot_fail_wrapper("Read USB error");
		fastboot_state = STATE_ERROR;
		return;
	}
	download_size = len;

	dprintf(DBG_LV, "read OK.  data:0x%x\n", *((int*)download_base));
	fastboot_ok_wrapper("USB Transmission OK", len);
}
*/

void cmd_flash_emmc_img(const char *arg, void *data, unsigned sz)
{
	unsigned long long ptn = 0;
	unsigned long long size = 0;
	unsigned long long size_wrote = 0;
	int index = INVALID_PTN;
	u32 pre_chksum = 0;
	u32 post_chksum = 0;
	char msg[256];
	dprintf(DBG_LV, "Function cmd_flash_img()\n");
	dprintf(DBG_LV, "EMMC Offset[0x%x], Length[0x%x], data In[0x%x]\n", arg, sz, data);
	TIME_START;

	if (!strcmp(arg, "partition"))
	{
		dprintf(DBG_LV, "Attempt to write partition image.(MBR, GPT?)\n");
		dprintf(DBG_LV, "Not supported, return.\n");
		fastboot_fail_wrapper("Not supported 'partition'.\n");
		return;
		/*if (write_partition(sz, (unsigned char *) data)) {
		fastboot_fail_wrapper("failed to write partition");
		return;
		}*/
	}
	else
	{
		index = partition_get_index(arg);
		if(index == -1)
		{
			fastboot_fail_wrapper("partition table doesn't exist");
			return;
		}
		if(!is_support_flash(index)){
			sprintf(msg,"\npartition '%s' not support flash",arg);
			fastboot_fail_wrapper(msg);
			return;
		}

		ptn = partition_get_offset(index);

		dprintf(DBG_LV, "[arg:%s]-[index:%d]-[ptn(offset):0x%x]\n", arg, index, ptn);

		if (!strcmp(arg, "boot") || !strcmp(arg, "recovery"))
		{
			if (memcmp((void *)data, BOOT_MAGIC, strlen(BOOT_MAGIC)))
			{
				fastboot_fail_wrapper("\nimage is not a boot image");
				return;
			}
		}

		size = partition_get_size(index);
		dprintf(DBG_LV, "[index:%d]-[partitionSize:%lld]-[downSize:%lld]\n", index, size, sz);

		if (ROUND_TO_PAGE(sz,511) > size)
		{
			fastboot_fail_wrapper("size too large");
			dprintf(DBG_LV, "size too large");
			return;
		}

		display_info("\nWriting Flash ... ");

		pre_chksum = calc_checksum(data,  (u32)sz);

		size_wrote = emmc_write(ptn , data, sz);
		if (size_wrote  != sz)
		{
			dprintf(DBG_LV, "emmc_write() Failed. act(%lld) != want(%lld)\n", size_wrote, sz);
			fastboot_fail_wrapper("\nFlash write failure");
			return;
		}

		if(sz != emmc_read(ptn,  data,  sz))
		{
			dprintf(DBG_LV, "emmc_read() Failed.\n");
			fastboot_fail_wrapper("\nRead EMMC error.");
			return ;
		}

		post_chksum = calc_checksum(data, (u32)sz);
		if(post_chksum != pre_chksum)
		{
			dprintf(DBG_LV, "write_to_emmc() Failed. checksum error\n");
			fastboot_fail_wrapper("\nChecksum Error.");
			return;
		}

		fastboot_ok_wrapper("OK", sz);
	}
	return;
}

void cmd_flash_emmc_sparse_img(const char *arg, void *data, unsigned sz)
{
	unsigned int chunk;
	unsigned int chunk_data_sz;
	sparse_header_t *sparse_header;
	chunk_header_t *chunk_header;
	u32 total_blocks = 0;
	unsigned long long ptn = 0;
	unsigned long long size = 0;
	unsigned long long size_wrote = 0;
	int index = INVALID_PTN;
	char msg[256];
	dprintf(DBG_LV, "Enter cmd_flash_sparse_img()\n");
	dprintf(DBG_LV, "EMMC Offset[0x%x], Length[%d], data In[0x%x]\n", arg, sz, data);

	index = partition_get_index(arg);
	if(index == -1)
	{
		fastboot_fail_wrapper("partition table doesn't exist");
		return;
	}

	if(!is_support_flash(index)){
		sprintf(msg,"partition '%s' not support flash\n",arg);
		fastboot_fail_wrapper(msg);
		return;
	}

	ptn = partition_get_offset(index);

	size = partition_get_size(index);
	if (ROUND_TO_PAGE(sz,511) > size)
	{
		fastboot_fail_wrapper("size too large");
		return;
	}

	/* Read and skip over sparse image header */
	sparse_header = (sparse_header_t *) data;
	data += sparse_header->file_hdr_sz;
	if(sparse_header->file_hdr_sz > sizeof(sparse_header_t))
	{
		/* Skip the remaining bytes in a header that is longer than
		* we expected.
		*/
		data += (sparse_header->file_hdr_sz - sizeof(sparse_header_t));
	}

	dprintf (DBG_LV, "=== Sparse Image Header ===\n");
	dprintf (DBG_LV, "magic: 0x%x\n", sparse_header->magic);
	dprintf (DBG_LV, "major_version: 0x%x\n", sparse_header->major_version);
	dprintf (DBG_LV, "minor_version: 0x%x\n", sparse_header->minor_version);
	dprintf (DBG_LV, "file_hdr_sz: %d\n", sparse_header->file_hdr_sz);
	dprintf (DBG_LV, "chunk_hdr_sz: %d\n", sparse_header->chunk_hdr_sz);
	dprintf (DBG_LV, "blk_sz: %d\n", sparse_header->blk_sz);
	dprintf (DBG_LV, "total_blks: %d\n", sparse_header->total_blks);
	dprintf (DBG_LV, "total_chunks: %d\n", sparse_header->total_chunks);

	TIME_START;
	display_info("Writing Flash ... ");
	/* Start processing chunks */
	for (chunk=0; chunk<sparse_header->total_chunks; chunk++)
	{
		/* Read and skip over chunk header */
		chunk_header = (chunk_header_t *) data;
		data += sizeof(chunk_header_t);

		dprintf (INFO, "=== Chunk Header ===\n");
		dprintf (INFO, "chunk_type: 0x%x\n", chunk_header->chunk_type);
		dprintf (INFO, "chunk_data_sz: 0x%x\n", chunk_header->chunk_sz);
		dprintf (INFO, "total_size: 0x%x\n", chunk_header->total_sz);

		if(sparse_header->chunk_hdr_sz > sizeof(chunk_header_t))
		{
			/* Skip the remaining bytes in a header that is longer than
			* we expected.
			*/
			data += (sparse_header->chunk_hdr_sz - sizeof(chunk_header_t));
		}

		chunk_data_sz = sparse_header->blk_sz * chunk_header->chunk_sz;
		switch (chunk_header->chunk_type)
		{
		case CHUNK_TYPE_RAW:
			if(chunk_header->total_sz != (sparse_header->chunk_hdr_sz +
				chunk_data_sz))
			{
				fastboot_fail_wrapper("Bogus chunk size for chunk type Raw");
				return;
			}


			dprintf(INFO, "[Flash Base Address:0x%llx offset:0x%llx]-[size:%d]-[DRAM Address:0x%x]\n",
				ptn , ((uint64_t)total_blocks*sparse_header->blk_sz), chunk_data_sz, data);

			size_wrote = emmc_write(ptn + ((uint64_t)total_blocks*sparse_header->blk_sz),
				(unsigned int*)data, chunk_data_sz);

			dprintf(INFO, "[wrote:%lld]-[size:%d]\n", size_wrote ,chunk_data_sz);

			if(size_wrote != chunk_data_sz)
			{
				fastboot_fail_wrapper("flash write failure");
				return;
			}
			total_blocks += chunk_header->chunk_sz;
			data += chunk_data_sz;
			break;

		case CHUNK_TYPE_DONT_CARE:
			total_blocks += chunk_header->chunk_sz;
			break;

		case CHUNK_TYPE_CRC:
			if(chunk_header->total_sz != sparse_header->chunk_hdr_sz)
			{
				fastboot_fail_wrapper("Bogus chunk size for chunk type Dont Care");
				return;
			}
			total_blocks += chunk_header->chunk_sz;
			data += chunk_data_sz;
			break;

		default:
			fastboot_fail_wrapper("Unknown chunk type");
			return;
		}
	}

	dprintf(DBG_LV, "Wrote %d blocks, expected to write %d blocks\n",
		total_blocks, sparse_header->total_blks);

	if(total_blocks != sparse_header->total_blks)
	{
		fastboot_fail_wrapper("sparse image write failure");

	}
	else
	{
		fastboot_ok_wrapper("Write Flash OK", sz);
	}


	return;
}
void cmd_flash_emmc(const char *arg, void *data, unsigned sz)
{
	sparse_header_t *sparse_header;
	/* 8 Byte Magic + 2048 Byte xml + Encrypted Data */
	unsigned int *magic_number = (unsigned int *) data;
	int ret=0;
#if 0
	if (magic_number[0] == DECRYPT_MAGIC_0 &&
		magic_number[1] == DECRYPT_MAGIC_1)
	{
#ifdef SSD_ENABLE
		ret = decrypt_scm((u32 **) &data, &sz);
#endif
		if (ret != 0)
		{
			dprintf(CRITICAL, "ERROR: Invalid secure image\n");
			return;
		}
	}
	else if (magic_number[0] == ENCRYPT_MAGIC_0 &&
		magic_number[1] == ENCRYPT_MAGIC_1)
	{
#ifdef SSD_ENABLE
		ret = encrypt_scm((u32 **) &data, &sz);
#endif
		if (ret != 0)
		{
			dprintf(CRITICAL, "ERROR: Encryption Failure\n");
			return;
		}
	}
#endif
	if(sz  == 0)
	{
		fastboot_okay("");
		return;
	}

	sparse_header = (sparse_header_t *) data;

	dprintf(DBG_LV, "cmd_flash_emmc, data:0x%x,, n sparse_header->magic = 0x%x\n",*(int*)data, sparse_header->magic );

	if (sparse_header->magic != SPARSE_HEADER_MAGIC)
	{
		cmd_flash_emmc_img(arg, data, sz);
	}
	else
	{
		cmd_flash_emmc_sparse_img(arg, data, sz);
	}
	return;
}

void cmd_erase_emmc(const char *arg, void *data, unsigned sz)
{
	unsigned long long ptn = 0;
	unsigned long long size = 0;
	int index = INVALID_PTN;
	int erase_ret = MMC_ERR_NONE;
	char msg[256];

	init_display_xy();
	dprintf (DBG_LV, "Enter cmd_erase()\n");

	index = partition_get_index(arg);
	ptn = partition_get_offset(index);

	if(index == -1) {
		fastboot_fail_wrapper("Partition table doesn't exist");
		return;
	}
	if(!is_support_erase(index)){
		sprintf(msg,"partition '%s' not support erase\n",arg);
		fastboot_fail_wrapper(msg);
		return;
	}

	TIME_START;
	size = partition_get_size(index);
	erase_ret = emmc_erase(ptn, partition_get_size(index));

	if(erase_ret  == MMC_ERR_NONE)
	{
		dprintf (DBG_LV, "emmc_erase() OK\n");
		fastboot_ok_wrapper("Erase EMMC", size);
	}
	else
	{
		dprintf (DBG_LV, "emmc_erase() Fail\n");
		snprintf(msg, sizeof(msg), "Erase error. code:%d", erase_ret);
		fastboot_fail_wrapper(msg);
	}


	return;
}
void cmd_flash_nand(const char *arg, void *data, unsigned sz)
{
	int index;
	u64 offset,size;
	bool partition_type;
	char *p_type;
	char msg[256];

	if(sz  == 0)
	{
		fastboot_okay("");
		return;
	}

	index = partition_get_index(arg);
	if(index == -1){
		fastboot_fail_wrapper("partition get index fail");
		return;
	}
	if(!is_support_flash(index)){
		sprintf(msg,"partition '%s' not support flash\n",arg);
		fastboot_fail_wrapper(msg);
		return;
	}

	offset = partition_get_offset(index);
	if(offset == -1){
		fastboot_fail_wrapper("partition get offset fail");
		return;
	}else{
		printf("get offset: 0x%llx\n",offset);
	}
	size = partition_get_size(index);
	if(size == -1){
		fastboot_fail_wrapper("partition get size fail");
		return;
	}else{
		printf("get size: 0x%llx\n",size);
	}

	if (!strcmp(arg, "boot") || !strcmp(arg, "recovery"))
	{
		if (memcmp((void *)data, BOOT_MAGIC, strlen(BOOT_MAGIC)))
		{
			fastboot_fail_wrapper("image is not a boot image");
			return;
		}
	}
    partition_get_type(index,&p_type);
	partition_type = (!strcmp(p_type,"yaffs2"))?1:0;
	TIME_START;
	display_info("write flash ....");
	printf("writing %d bytes to '%s' '%s' partition_type %d\n", sz, arg,p_type,partition_type);
	if (nand_write_img((u32)offset, data, sz,(u32)size,partition_type)) {
		fastboot_fail_wrapper("nand  write image failure");
		return;
	}
	printf("partition '%s' updated\n", arg);
	fastboot_ok_wrapper("write flash sucess",sz);
	return;
}

void cmd_erase_nand(const char *arg, void *data, unsigned sz)
{

	int index;
	u64 offset,size;
	char msg[256];
	index = partition_get_index(arg);
	if(index == -1){
		fastboot_fail_wrapper("partition get index fail");
		return;
	}
	if(!is_support_erase(index)){
		sprintf(msg,"partition '%s' not support erase\n",arg);
		fastboot_fail_wrapper(msg);
		return;
	}

	offset = partition_get_offset(index);
	if(offset == -1){
		fastboot_fail_wrapper("partition get offset fail");
		return;
	}else{
		printf("get offset: 0x%llx\n",offset);
	}
	size = partition_get_size(index);
	if(size == -1){
		fastboot_fail_wrapper("partition get size fail");
		return;
	}else{
		printf("get size: 0x%llx\n",size);
	}

	TIME_START;
	display_info("erase flash ....");

	if(nand_erase(offset,size)!=0){
		fastboot_fail_wrapper("failed to erase partition");
		return;
	}

	fastboot_ok_wrapper("erase flash sucess",sz);

	return;
}


static void init_display_xy()
{
#if defined(DISPLAY_INFO_ON_LCM)
	video_clean_screen();
	video_set_cursor(video_get_rows()/2, 0);
	//video_set_cursor(1, 0);
#endif
}

static void display_info(const char* msg)
{
#if defined(DISPLAY_INFO_ON_LCM)
	if(msg == 0)
	{
		return;
	}
	video_printf("%s\n", msg);
#endif
}

static void display_progress(const char* msg_prefix, unsigned size, unsigned totle_size)
{
#if defined(DISPLAY_INFO_ON_LCM)
	unsigned vel = 0;
	unsigned prog = 0;
	unsigned time = TIME_ELAPSE;

	if(msg_prefix == 0)
	{
		msg_prefix = "Unknown";
	}

	if(time != 0)
	{
		vel = (unsigned)(size / time); //approximate  1024/1000
		time /= 1000;
	}
	if(totle_size != 0)
	{
		prog = (unsigned)((size*100.0f)/totle_size);
	}
	video_printf("%s > %3d%% Time:%4ds Vel:%5dKB/s", msg_prefix, prog, time, vel);
#endif
}

static void display_speed_info(const char* msg_prefix, unsigned size)
{
#if defined(DISPLAY_INFO_ON_LCM)
	unsigned vel = 0;
	unsigned time = TIME_ELAPSE;

	if(msg_prefix == 0)
	{
		msg_prefix = "Unknown";
	}

	if(time != 0)
	{
		vel = (unsigned)(size / time); //approximate  1024/1000
	}
	video_printf("%s  Time:%dms Vel:%dKB/s \n", msg_prefix, time, vel);
#endif
}

static void fastboot_fail_wrapper(const char* msg)
{
	display_info(msg);
	fastboot_fail(msg);
}

static void fastboot_ok_wrapper(const char* msg, unsigned data_size)
{
	display_speed_info(msg, data_size);
	fastboot_okay("");
}

static unsigned hex2unsigned(const char *x)
{
    unsigned n = 0;

    while(*x) {
        switch(*x) {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            n = (n << 4) | (*x - '0');
            break;
        case 'a': case 'b': case 'c':
        case 'd': case 'e': case 'f':
            n = (n << 4) | (*x - 'a' + 10);
            break;
        case 'A': case 'B': case 'C':
        case 'D': case 'E': case 'F':
            n = (n << 4) | (*x - 'A' + 10);
            break;
        default:
            return n;
        }
        x++;
    }

    return n;
}

/*LXO: END!Download related command*/
