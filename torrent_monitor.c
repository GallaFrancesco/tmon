#include <stdio.h>
#include <stdlib.h>
#include <linux/inotify.h>
#include <poll.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>
#include "string_utils.h"
#include "settings.h"

#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024*(EVENT_SIZE + 256))
static int verbose_flag = 0;

static void skeleton_daemon(char *dir)
{
    pid_t pid;
    int x;
    
	/* Fork off the parent process */
    pid = fork();
    /* An error occurred */
    if (pid < 0){
        exit(EXIT_FAILURE);
	} else if(pid > 0) {
        exit(EXIT_SUCCESS);
	}
    /* On success: The child process becomes session leader */
    if (setsid() < 0){
        exit(EXIT_FAILURE);
	}
    /* Catch, ignore and handle signals */
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
	/* Fork off for the second time*/
    pid = fork();

    /* An error occurred */
    if (pid < 0){
        exit(EXIT_FAILURE);
	} else if(pid > 0){
        exit(EXIT_SUCCESS);
	}
    /* Set new file permissions */
    umask(0);

    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    chdir(dir);

    /* Close all open file descriptors */
    for (x = sysconf(_SC_OPEN_MAX); x>0; x--)
    {
        close (x);
    }
    /* Open the log file */
    openlog ("tmon", LOG_PID, LOG_DAEMON);
}

/*sends the command to transmission-daemon*/
void add_torrent(const char *torname, char *tordir)
{
	FILE *out;
	char *command = NULL;

	command = build_command(tordir, torname, verbose_flag);
	if(!command){
		fprintf(stderr, "Unable to build command.\n");
		exit(EXIT_FAILURE);
	} 
	
	if(verbose_flag){
		fprintf(stdout, "Command:\n%s\n", command);
	}
	out = popen(command, "w");
	free(command);
	pclose(out);
	return;
}

/*it handles events received during polling (see main)*/
void handle_events(int fd, int wd, char *tordir){
	
	char buf[EVENT_BUF_LEN];
		//__attribute__ ((aligned(__alignof__(struct inotify_event))));
	const struct inotify_event *event;
	int i;
	ssize_t len;
	char *ptr;

	/*loop reading events*/
	while(1){
		len = read(fd, buf, sizeof buf);
		if(len == -1 && errno != EAGAIN){
			perror("Read");
			exit(EXIT_FAILURE);
		}

		if(len<0){
			break;
		}

		/*loop over all events in buffer*/
		for(ptr = buf; ptr<buf+len; ptr+= sizeof(struct inotify_event)+event->len){
			event = (const struct inotify_event*) ptr;

			if(event->len){
			//	fprintf(stdout, "File %s added.\n", event->name);
				sleep(2); // since torrents opened unfinished cause problems
				add_torrent(event->name, tordir);	
			}
		}
	}
	return;
}

int main(int argc, char*argv[])
{
	char *torrent_dir;
	int fd, wd, poll_num;
	int len, i = 0;

	char buffer[EVENT_BUF_LEN];
	struct pollfd *fds = NULL;
	nfds_t nfds;

	if(argc != 3 && strcmp(argv[1], "-h") != 0 && strcmp(argv[1], "--help") != 0){
		fprintf(stderr, "Usage: ./tmon [torrent directory]\nAt the moment, MUSIC, FILM and MAINDIR are hardcoded definitions\nof the folders in which you want to send your files to.\nTo modify them, open torrent_monitor.c and define different paths.\nFor more detailed usage, run with -h, --help flag.\n");
		exit(EXIT_FAILURE);
	}
	
	/*daemonize or verbose*/
	if(strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "--daemon") == 0){
		printf("%s\n", argv[2]);
		skeleton_daemon(argv[2]);
	} else if(strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--verbose") == 0){
		fprintf(stdout, "Starting torrent-monitor in verbose mode.\nThe process won't become a daemon.\n");
		verbose_flag = 1;
	} else if(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0){
		fprintf(stdout, "Usage: ./tmon {args} [torrent directory]\n\nTmon monitors the directory passed as a command line argment and, as soon as a torrent file is added, it sends it to transmission-daemon with directory based on the filetype of the torrent.\nAt the moment, MUSIC, FILM and MAINDIR are hardcoded definitions\nof the folders in which you want to send your files to.\nTo modify them, open torrent_monitor.c and define different paths.\nWant to check if the daemon is running?\nTry: ps -xj | grep -i tmon\n");
		fprintf(stdout, "Arguments:\n\t-v, --verbose\t\tStart in verbose mode\n\t-d, --daemon\t\tStart in daemon mode\n\t-h, --help\t\tPrint this help and exit\n");
		exit(EXIT_SUCCESS);
	}
	
	/*get torrent_dir from argv*/
	torrent_dir = alloc_dir(argv[2]);
	if(verbose_flag){
		fprintf(stdout, "Monitoring directory: %s\nStarting inotify...\n", torrent_dir);	
	}

	/*get a file descriptor*/
	fd = inotify_init(IN_NONBLOCK);
	if(fd < 0){
		fprintf(stderr, "Could not get a file descriptor for \"inotify_init\".\n");
		exit(EXIT_FAILURE);
	}
	
	/*add the torrent_dir to the watchlist	*/
	wd = inotify_add_watch(fd, torrent_dir, IN_MOVED_TO | IN_CLOSE_WRITE); //IN_MOVED_TO checks for new files
														  //moved into the directory
	if(verbose_flag){
		fprintf(stdout, "Directory %s added to watchlist.\n", torrent_dir);	
	}
	
	nfds = 1;
	fds = (struct pollfd*)malloc(sizeof(struct pollfd)); //file descriptors
	fds->fd = fd;										 //PROTIP: actually integers
	fds->events = POLLIN;
	
	if(verbose_flag){
		fprintf(stdout, "Started polling, waiting for events...\n", torrent_dir);	
	}
	/*wait for events*/
	while(1){
		poll_num = poll(fds, nfds, -1);
		
		if(poll_num <= 0){
			perror("Poll"); //Yet too many details, this basically means no polling possible
			exit(EXIT_FAILURE);
		}

		if(poll_num > 0){ //That's right
			handle_events(fd, wd, torrent_dir);
		}
	}
	
	inotify_rm_watch(fd, wd);
	close(fd);
	free(fds);
	free(torrent_dir);
	
	return 0;
}

