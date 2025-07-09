#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include "es5341_do_cfg.h"
#include <vi53xx_api.h>

int verbose = 0;

uint64_t getopt_integer(char *optarg)
{
        int rc; 
        uint64_t value;

        rc = sscanf(optarg, "0x%lx", &value);
        if (rc <= 0)
                rc = sscanf(optarg, "%lu", &value);

        return value;
}

static struct option const long_opts[] = {
	{"DO Config IP", required_argument, NULL, 'c'},
	{"DO_en", required_argument, NULL, 'e'},
	{"DO_ref", required_argument, NULL, 'r'},
	{"help", no_argument, NULL, 'h'},
	{"verbose", no_argument, NULL, 'v'},
	{0, 0, 0, 0}
};

static int do_config(int ch, unsigned int en, unsigned int ref);

static void usage(const char *name)
{
	int i = 0;
	int value = 0;

	fprintf(stdout, "%s\n\n", name);
	fprintf(stdout, "usage: %s [OPTIONS]\n\n", name);

	fprintf(stdout, "  -%c (--%s) DO channel (defaults to %d)\n",
		long_opts[i].val, long_opts[i].name, value);
	i++;
	fprintf(stdout, "  -%c (--%s) DOn_En (defaults to %d)\n",
	       long_opts[i].val, long_opts[i].name, value);
	i++;
	fprintf(stdout, "  -%c (--%s) DOn_Ref (defaults to %d)\n",
	       long_opts[i].val, long_opts[i].name, value);
	i++;
	fprintf(stdout,
		"  -%c (--%s) help.\n",
		long_opts[i].val, long_opts[i].name );
	i++;
	fprintf(stdout, "  -%c (--%s)  log\n",
		long_opts[i].val, long_opts[i].name);
}

int main(int argc, char *argv[])
{
	int cmd_opt;
	int  ch = 0;
	unsigned int  en = 0;
	unsigned int  ref = 0;

	while ((cmd_opt = getopt_long(argc, argv, "vh:c:e:r:", long_opts,
			    NULL)) != -1) {
		switch (cmd_opt) {
		case 0:
			/* long option */
			break;
		case 'c':
			/* DO channel */
			ch = getopt_integer(optarg);
			break;
		case 'e':
			/* DO En */
			en = getopt_integer(optarg);
			break;
		case 'r':
			/* DO Ref */
			ref = getopt_integer(optarg);
			break;
		case 'v':
			verbose = 1;
			break;
		case 'h':
		default:
			usage(argv[0]);
			exit(0);
			break;
		}
	}
	if (verbose)
	fprintf(stdout,
		"DO Config IP  DO%d_En = 0x%x,  DO%d_Ref = 0x%x\n", ch, en, ch, ref);

	return do_config(ch, en, ref);
}

static int do_config(int ch, unsigned int en, unsigned int ref)
{
	int rc;

	handler_do_config(1, ch, en, ref);

	return rc;
}
