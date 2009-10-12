/* radare - LGPL - Copyright 2009 pancake<nopcode.org> */

#include <r_util.h>
#include <r_debug.h>
#include <r_io.h>

int main(int argc, char **argv)
{
	int ret;
	int tid, pid;
	struct r_io_t *io;
	struct r_debug_t *dbg;

	io = r_io_new();
	printf("Supported IO pluggins:\n");
	r_io_handle_list(io);

	ret = r_io_open(io, "dbg:///bin/ls", 0, 0);
//	r_io_set_fd(io, ret);
	printf("r_io_open dbg:///bin/ls' = %s\n", r_str_bool(ret));
	if (!ret) {
		printf("Cannot open dbg:///bin/ls\n");
		goto beach;
	}

	{
		/* dump process memory */
		ut8 buf[128];
		int ret = r_io_read_at(io, 0x8048000, buf, 128);
		printf("%d : %02x\n", ret, buf[0]);
	}

	dbg = r_debug_new();
	printf("Supported debugger backends:\n");

	ret = r_debug_use(dbg, "ptrace");
	printf("Using dbg ptrace = %s\n", r_str_bool(ret));
	
	tid = pid = r_io_system(io, "pid");
	r_debug_select(dbg, pid, tid);

	printf("--> regs pre step\n");
	r_io_system(io, "reg");

	printf("--> perform 2 steps (only 1 probably?)\n");
	r_debug_step(dbg, 2);
	// TODO: this is not working :/
	r_debug_reg_sync(dbg, R_REG_TYPE_GPR, 0);
	r_debug_reg_list(dbg, R_REG_TYPE_GPR, 32, 0);

	printf("--> regs post step\n");
	r_io_system(io, "reg");

	printf("---\n");
	r_debug_continue (dbg);
	printf("---\n");

beach:
	r_io_free(io);
	r_debug_free(dbg);
	return 0;
}