#include <debug.h>
#include <string.h>
#include <stdlib.h>
#include <kernel/thread.h>
#include <kernel/event.h>
#include <dev/udc.h>
#include "sys_commands.h"
#include "download_commands.h"
#include "fastboot.h"
#include <mt_partition.h>

#define MAX_RSP_SIZE 64
#define MAX_USBFS_BULK_SIZE (16 * 1024)

#define EXPAND(NAME) #NAME
#define TARGET(NAME) EXPAND(NAME)

static event_t usb_online;
static event_t txn_done;
static unsigned char buffer[4096] __attribute__((aligned(32)));
static struct udc_endpoint *in, *out;
static struct udc_request *req;
int txn_status;

void *download_base;
unsigned download_max;
unsigned download_size;

unsigned fastboot_state = STATE_OFFLINE;

struct fastboot_cmd *cmdlist;

static void req_complete(struct udc_request *req, unsigned actual, int status)
{
	txn_status = status;
	req->length = actual;
	event_signal(&txn_done, 0);
}


void fastboot_register(const char *prefix,
		       void (*handle)(const char *arg, void *data, unsigned sz))
{
	struct fastboot_cmd *cmd;
	cmd = malloc(sizeof(*cmd));
	if (cmd) {
		cmd->prefix = prefix;
		cmd->prefix_len = strlen(prefix);
		cmd->handle = handle;
		cmd->next = cmdlist;
		cmdlist = cmd;
	}
}

struct fastboot_var *varlist;

void fastboot_publish(const char *name, const char *value)
{
	struct fastboot_var *var;
	var = malloc(sizeof(*var));
	if (var) {
		var->name = name;
		var->value = value;
		var->next = varlist;
		varlist = var;
	}
}

int usb_read(void *_buf, unsigned len)
{
	int r;
	unsigned xfer;
	unsigned char *buf = _buf;
	int count = 0;

	if (fastboot_state == STATE_ERROR)
		goto oops;

	while (len > 0) {
		xfer = (len > MAX_USBFS_BULK_SIZE) ? MAX_USBFS_BULK_SIZE : len;
		req->buf = buf;
		req->length = xfer;
		req->complete = req_complete;
		r = udc_request_queue(out, req);
		if (r < 0) {
			dprintf(INFO, "usb_read() queue failed\n");
			goto oops;
		}
		event_wait(&txn_done);

		if (txn_status < 0) {
			dprintf(INFO, "usb_read() transaction failed\n");
			goto oops;
		}

		count += req->length;
		buf += req->length;
		len -= req->length;

		/* short transfer? */
		if (req->length != xfer) break;
	}

	return count;

oops:
	fastboot_state = STATE_ERROR;
	return -1;
}

int usb_write(void *buf, unsigned len)
{
	int r;

	if (fastboot_state == STATE_ERROR)
		goto oops;

	req->buf = buf;
	req->length = len;
	req->complete = req_complete;
	r = udc_request_queue(in, req);
	if (r < 0) {
		dprintf(INFO, "usb_write() queue failed\n");
		goto oops;
	}
	event_wait(&txn_done);
	if (txn_status < 0) {
		dprintf(INFO, "usb_write() transaction failed\n");
		goto oops;
	}
	return req->length;

oops:
	fastboot_state = STATE_ERROR;
	return -1;
}

void fastboot_ack(const char *code, const char *reason)
{
	char response[MAX_RSP_SIZE];

	if (fastboot_state != STATE_COMMAND)
		return;

	if (reason == 0)
		reason = "";

	snprintf(response, MAX_RSP_SIZE, "%s%s", code, reason);
	fastboot_state = STATE_COMPLETE;

	usb_write(response, strlen(response));

}

void fastboot_info(const char *reason)
{
	char response[MAX_RSP_SIZE];

	if (fastboot_state != STATE_COMMAND)
		return;

	if (reason == 0)
		return;

	snprintf(response, MAX_RSP_SIZE, "INFO%s", reason);

	usb_write(response, strlen(response));
}

void fastboot_fail(const char *reason)
{
	fastboot_ack("FAIL", reason);
}

void fastboot_okay(const char *info)
{
	fastboot_ack("OKAY", info);
}

static void fastboot_command_loop(void)
{
	struct fastboot_cmd *cmd;
	int r;
	dprintf(ALWAYS,"fastboot: processing commands\n");

again:
	while (fastboot_state != STATE_ERROR)
	{
		memset(buffer, 0, sizeof(buffer));
		r = usb_read(buffer, MAX_RSP_SIZE);
		if (r < 0) break; //no input command
		buffer[r] = 0;
		dprintf(ALWAYS,"[fastboot: command buf]-[%s]-[len=%d]\n", buffer, r);
		dprintf(ALWAYS,"[fastboot]-[download_base:0x%x]-[download_size:0x%x]\n", download_base, download_size);

		/*Pick up matched command and handle it*/
		for (cmd = cmdlist; cmd; cmd = cmd->next)
		{
			if (memcmp(buffer, cmd->prefix, cmd->prefix_len))
			{
				continue;
			}

			fastboot_state = STATE_COMMAND;
			dprintf(ALWAYS,"[Cmd process]-[buf:%s]-[lenBuf:%s]\n", buffer,  buffer + cmd->prefix_len);
			cmd->handle((const char*) buffer + cmd->prefix_len,
				(void*) download_base, download_size);

			if (fastboot_state == STATE_COMMAND)
			{
				fastboot_fail("unknown reason");
			}
			goto again;
		}
		dprintf(ALWAYS,"[unknown command]*[%s]*\n", buffer);
		fastboot_fail("unknown command");

	}
	fastboot_state = STATE_OFFLINE;
	dprintf(ALWAYS,"fastboot: oops!\n");
}

static int fastboot_handler(void *arg)
{
	for (;;) {
		event_wait(&usb_online);
		fastboot_command_loop();
	}
	return 0;
}

static void fastboot_notify(struct udc_gadget *gadget, unsigned event)
{
	if (event == UDC_EVENT_ONLINE) {
		event_signal(&usb_online, 0);
	}
}

static struct udc_endpoint *fastboot_endpoints[2];

static struct udc_gadget fastboot_gadget = {
	.notify		= fastboot_notify,
	.ifc_class	= 0xff,
	.ifc_subclass	= 0x42,
	.ifc_protocol	= 0x03,
	.ifc_endpoints	= 2,
	.ifc_string	= "fastboot",
	.ept		= fastboot_endpoints,
};

extern void fastboot_oem_register();
static void register_parition_var(void)
{
	int i;
	unsigned long long p_size;
	char *type_buf;
	char *value_buf;
	char *var_name_buf;
	char *p_name_buf;

	for(i=0;i<PART_MAX_COUNT;i++){
		p_size = partition_get_size(i);
		if(p_size == -1)
			break;
		partition_get_name(i,&p_name_buf);

		partition_get_type(i,&type_buf);
		var_name_buf = malloc(30);
		sprintf(var_name_buf,"partition-type:%s",p_name_buf);
		fastboot_publish(var_name_buf,type_buf);
		//printf("%d %s %s\n",i,var_name_buf,type_buf);

		/*reserved for MTK security*/
		if(!strcmp(type_buf,"ext4")){
			if(!strcmp(p_name_buf,"userdata")){
				p_size -= (u64)2*1024*1024;
			}else{
				p_size -= (u64)1*1024*1024;
			}
		}
		value_buf = malloc(20);
		sprintf(value_buf,"%llx",p_size);
		var_name_buf = malloc(30);
		sprintf(var_name_buf,"partition-size:%s",p_name_buf);
		fastboot_publish(var_name_buf,value_buf);
		//printf("%d %s %s\n",i,var_name_buf,value_buf);
		
	}
}
int fastboot_init(void *base, unsigned size)
{
	thread_t *thr;

	dprintf(ALWAYS, "fastboot_init()\n");

	download_base = base;
	download_max = size;

	mtk_wdt_disable(); /*It will re-enable during continue boot*/

	fastboot_register("getvar:", cmd_getvar);
	fastboot_publish("version", "0.5");
	fastboot_register("boot", cmd_boot);

	/*Hong-Rong: wait for porting*/

#ifdef MTK_EMMC_SUPPORT
	fastboot_register("flash:", cmd_flash_emmc);
	fastboot_register("erase:", cmd_erase_emmc);
#else
	fastboot_register("flash:", cmd_flash_nand);
	fastboot_register("erase:", cmd_erase_nand);
#endif

	fastboot_register("continue", cmd_continue);
	fastboot_register("reboot", cmd_reboot);
	fastboot_register("reboot-bootloader", cmd_reboot_bootloader);
	fastboot_publish("product", TARGET(BOARD));
	fastboot_publish("kernel", "lk");
	//fastboot_publish("serialno", sn_buf);

	register_parition_var();


	/*LXO: Download related command*/
	fastboot_register("download:", cmd_download);

	/*LXO: END!Download related command*/

	fastboot_oem_register();

	event_init(&usb_online, 0, EVENT_FLAG_AUTOUNSIGNAL);
	event_init(&txn_done, 0, EVENT_FLAG_AUTOUNSIGNAL);

	in = udc_endpoint_alloc(UDC_TYPE_BULK_IN, 512);
	if (!in)
		goto fail_alloc_in;
	out = udc_endpoint_alloc(UDC_TYPE_BULK_OUT, 512);
	if (!out)
		goto fail_alloc_out;

	fastboot_endpoints[0] = in;
	fastboot_endpoints[1] = out;

	req = udc_request_alloc();
	if (!req)
		goto fail_alloc_req;

	if (udc_register_gadget(&fastboot_gadget))
		goto fail_udc_register;

	thr = thread_create("fastboot", fastboot_handler, 0, DEFAULT_PRIORITY, 4096);
	if (!thr)
	{
		goto fail_alloc_in;
	}
	thread_resume(thr);
	return 0;

fail_udc_register:
	udc_request_free(req);
fail_alloc_req:
	udc_endpoint_free(out);
fail_alloc_out:
	udc_endpoint_free(in);
fail_alloc_in:
	return -1;
}

