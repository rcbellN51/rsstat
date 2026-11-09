#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

#include <minix/endpoint.h>
#include <minix/type.h>
#include <minix/const.h>
#include <minix/com.h>
#include <minix/syslib.h>
#include <minix/sysinfo.h>
#include <minix/sysutil.h>
#include <minix/rs.h>
#include <machine/archtypes.h>
#include <minix/timers.h>

#include "/usr/src/minix/kernel/priv.h"
#include "/usr/src/minix/servers/rs/const.h"
#include "/usr/src/minix/servers/rs/type.h"

static struct rprocpub rprocpub[NR_SYS_PROCS];
static struct rproc rproc[NR_SYS_PROCS];

static const char *
flags_string(unsigned flags, unsigned sys_flags)
{
	static char str[7];

	str[0] = (flags & RS_ACTIVE) ? 'A' : '-';
	str[1] = (flags & RS_UPDATING) ? 'U' : '-';
	str[2] = (flags & RS_EXITING) ? 'E' : '-';
	str[3] = (flags & RS_NOPINGREPLY) ? 'N' : '-';
	str[4] = (sys_flags & SF_USE_COPY) ? 'C' : '-';
	str[5] = (sys_flags & SF_USE_REPL) ? 'R' : '-';
	str[6] = '\0';

	return str;
}

static void
print_help(void)
{
	fprintf(stdout, "Usage: rsstat [-h | --help] [service]\n");
	fprintf(stdout, "\n");
	fprintf(stdout, "Display MINIX 3 Reincarnation Server status.\n");
	fprintf(stdout, "\n");
	fprintf(stdout, "Options:\n");
	fprintf(stdout, "  -h, --help    Show this help message\n");
	fprintf(stdout, "\n");
	fprintf(stdout, "Arguments:\n");
	fprintf(stdout, "  service       Display only the named service\n");
}

int
main(int argc, char *argv[])
{
	struct rproc *rp;
	struct rprocpub *rpub;
	int r, i, count = 0;
	int service_width = 13;
	clock_t now;
	long hz;

	if (argc == 2 &&
		(strcmp(argv[1], "-h") == 0 ||
		strcmp(argv[1], "--help") == 0)) {
			print_help();
			return EXIT_SUCCESS;
	}

	if (argc > 2) {
		fprintf(stderr, "Usage: rsstat [-h | --help] [service]\n");
		return EXIT_FAILURE;
	}

	hz = sysconf(_SC_CLK_TCK);

	if (hz <= 0) {
		fprintf(stderr, "rsstat: unable to determine clock frequency\n");
		return EXIT_FAILURE;
	}

	r = getsysinfo(RS_PROC_NR, SI_PROCPUB_TAB,
	    rprocpub, sizeof(rprocpub));

	if (r != 0) {
		fprintf(stderr, "rsstat: public table request failed: %d\n", r);
		return EXIT_FAILURE;
	}

	r = getsysinfo(RS_PROC_NR, SI_PROC_TAB,
	    rproc, sizeof(rproc));

	if (r != 0) {
		fprintf(stderr, "rsstat: private table request failed: %d\n", r);
		return EXIT_FAILURE;
	}

	now = getticks();

	for (i = 0; i < NR_SYS_PROCS; i++) {
		int len;

		if (!(rproc[i].r_flags & RS_IN_USE))
			continue;

		if (argc == 2 && strcmp(rprocpub[i].label, argv[1]) != 0)
			continue;

		len = (int)strlen(rprocpub[i].label);

		if (len > service_width)
			service_width = len;
	}

	for (i = 0; i < NR_SYS_PROCS; i++) {
		rp = &rproc[i];
		rpub = &rprocpub[i];

		if (!(rp->r_flags & RS_IN_USE))
			continue;

		if (argc == 2 && strcmp(rpub->label, argv[1]) != 0)
			continue;

		if (count == 0) {
			printf("%-*s %9s %5s %6s %-6s %4s %9s %9s %8s %s\n",
				service_width,
				"SERVICE", "ENDPOINT", "PID", "FLAGS", "POLICY",
				"DEV", "PERIOD(s)", "HB_AGE(s)", "RESTARTS", "COMMAND");
		}

		printf("%-*s %9d %5d %6s %-6s %4d ",
			service_width,
			rpub->label,
			rpub->endpoint,
			rp->r_pid,
			flags_string(rp->r_flags, rpub->sys_flags),
			(rpub->sys_flags & SF_NORESTART) ? "NO" : "AUTO",
			rpub->dev_nr);

		if (rp->r_period > 0) {
			uint32_t elapsed;

			elapsed = (uint32_t)now -
				(uint32_t)rp->r_alive_tm;

			printf("%9.1f %9.1f ",
				(double)rp->r_period / hz,
				(double)elapsed / hz);
		} else {
			printf("%9s %9s ", "-", "-");
		}

		printf("%8d %s\n",
			rp->r_restarts,
			rp->r_args);

		count++;
	}

	if (argc == 2 && count == 0) {
		fprintf(stderr, "rsstat: service '%s' not found\n", argv[1]);
		return EXIT_FAILURE;
	}

	if (argc == 1)
		printf("\n%d registered services\n", count);

	return EXIT_SUCCESS;
}
