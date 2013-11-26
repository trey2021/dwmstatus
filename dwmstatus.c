#define _BSD_SOURCE
#include "alsavolume.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <X11/Xlib.h>

#define UPDATE_PERIOD 60
#define CONNECTION_INFO_COMMAND "~/scripts/dwmstatus-connection-info.sh"
#define CONNECTION_INFO_LEN 128

char *tzberlin = "Europe/Berlin";

static Display *dpy;

char *smprintf(char *fmt, ...)
{
	va_list fmtargs;
	char *ret;
	int len;

	va_start(fmtargs, fmt);
	len = vsnprintf(NULL, 0, fmt, fmtargs);
	va_end(fmtargs);

	ret = malloc(++len);
	if (ret == NULL) 
        {
		perror("malloc");
		exit(1);
	}

	va_start(fmtargs, fmt);
	vsnprintf(ret, len, fmt, fmtargs);
	va_end(fmtargs);

	return ret;
}

void settz(char *tzname)
{
	setenv("TZ", tzname, 1);
}

char *mktimes(char *fmt, char *tzname)
{
	char buf[129];
	time_t tim;
	struct tm *timtm;

	memset(buf, 0, sizeof(buf));
	settz(tzname);
	tim = time(NULL);
	timtm = localtime(&tim);

	if (timtm == NULL) 
        {
		perror("localtime");
		exit(1);
	}

	if (!strftime(buf, sizeof(buf)-1, fmt, timtm)) 
        {
		fprintf(stderr, "strftime == 0\n");
		exit(1);
	}

	return smprintf("%s", buf);
}

void setstatus(char *str)
{
	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);
}

char *loadavg(void)
{
	double avgs[3];

	if (getloadavg(avgs, 3) < 0) 
        {
		perror("getloadavg");
		exit(1);
	}

	return smprintf("%.2f %.2f %.2f", avgs[0], avgs[1], avgs[2]);
}

void get_connection_info(char *buf, size_t buf_len)
{
    FILE *pf;
    pf = popen(CONNECTION_INFO_COMMAND, "r");
    
    if (NULL == pf)
    {
         perror("connection info");
    }
    else
    {
        fgets(buf, buf_len, pf);
        pclose(pf);
    }
}

int main(void)
{
	char *status;
	char *avgs;
	char *tmbln;
        char coninfo[CONNECTION_INFO_LEN];
        long volume;
        
        dpy = XOpenDisplay(NULL);

	if (!dpy) 
        {
		fprintf(stderr, "dwmstatus: cannot open display.\n");
		return 1;
	}

	for (;;sleep(UPDATE_PERIOD)) 
        {
		avgs = loadavg();
                get_connection_info(coninfo, CONNECTION_INFO_LEN);
                volume = alsa_get_volume();
		tmbln = mktimes("%a %b %d %H:%M %Y", tzberlin);

		status = smprintf("%d% | %s | L:%s | %s",
				volume, coninfo, avgs, tmbln);

		setstatus(status);

		free(avgs);
		free(tmbln);
		free(status);
	}

	XCloseDisplay(dpy);

	return 0;
}

