#include "string_utils.h"

static int verbose_flag = 0;

/*allocates the working directory string*/
char *
alloc_dir(char *arg)
{
	char *dir;
	int len = strlen(arg);

	dir = (char*)calloc(len, sizeof(char));
	strcpy(dir, arg);
	return dir;
}

/*determines the ftype of torrent files*/
void 
parse_show(FILE *show, int *f_type)
{
	char line[400];
	char mp3[3] = "mp3";
	char mkv[3] = "mkv";
	char flac[4] = "flac";
	char ogg[3] = "ogg";
	char avi[3] = "avi";
	char mp4[3] = "mp4";
	
	if(fgets(line, 400, show) != NULL){
	
		if(strstr(line, mp3) != NULL || strstr(line, flac) != NULL || strstr(line, ogg) != NULL){
			if(verbose_flag){
				fprintf(stdout, "%s\n", line);
			}
			*f_type = 1;
			return;
		}
		if(strstr(line, mkv) != NULL || strstr(line, avi) != NULL || strstr(line, mp4) != NULL){
			if(verbose_flag){
				fprintf(stdout, "%s\n", line);	
			}
			*f_type = 0;
			return;
		}
	} else {
		if(verbose_flag){
			fprintf(stdout, "Unable to determine filetype. Sending to MAINDIR.\n");
		}
		return;
	}
	
	parse_show(show, f_type);
}

/*checks that the absolute working directory has a "/" at the end*/
void 
format_tordir(char *tor_dir_clean, int i, int last)
{
	if(tor_dir_clean[last-i] == '/'){
		if(i != 0){
			tor_dir_clean[last-i+1]	= '\0';
			return;
		} else {
			return;
		}
	}
	i++;
	format_tordir(tor_dir_clean, i, last);
}

/*welcome to string management hell*/
char *
build_command(char *tordir, const char *torname, int vf)
{
	char *command = NULL;
	char *torrent = NULL;
	char *tor_dir_clean = NULL;
	int tor_size;
	int comm_size;
	FILE *tor_file = NULL;
	char *show_command = NULL;
	int i, f_type = -1;

	verbose_flag = vf;
	tor_dir_clean = (char*)calloc((strlen(tordir)), sizeof(char));
	strcpy(tor_dir_clean, tordir);	
	format_tordir(tor_dir_clean, 0, strlen(tor_dir_clean) - 1);
	
	tor_size = strlen(torname) + strlen(tor_dir_clean);
	torrent = (char*)calloc(tor_size, sizeof(char));

	if(verbose_flag){
		fprintf(stdout, "tordir: %s, tor_dir_clean: %s, torname: %s\n", tordir, tor_dir_clean, torname);	
	}

	show_command = (char*)calloc((strlen("transmission-show ")+strlen(torname)+strlen(tor_dir_clean)), sizeof(char));
	strcpy(show_command, "transmission-show ");
	strcat(show_command, tor_dir_clean);
	strcat(show_command, torname);
	tor_file = popen(show_command, "r");
	if(!tor_file){
		exit(EXIT_FAILURE);
	}
	parse_show(tor_file, &f_type);

	if(f_type == 1){
		comm_size = strlen(torname)+strlen(tor_dir_clean)+strlen("transmission-remote -a ")+strlen(" -w ")+strlen(MUSIC);
	} else if(f_type == 0){
		comm_size = strlen(torname)+strlen(tor_dir_clean)+strlen("transmission-remote -a ")+strlen(" -w ")+strlen(FILM);
	} else {
		comm_size = strlen(torname)+strlen(tor_dir_clean)+strlen("transmission-remote -a ")+strlen(" -w ")+strlen(MAINDIR);	
	}
	
	command = (char*)calloc(comm_size, sizeof(char));
	strcpy(torrent, tor_dir_clean);
	strcat(torrent, torname);
	strcpy(command, "transmission-remote -a ");
	strcat(command, torrent);
	strcat(command, " -w ");
	if(f_type == 1){
		strcat(command, MUSIC);
	} else if(f_type == 0){
		strcat(command, FILM);
	} else {
		strcat(command, MAINDIR);	
	}
	//free(tor_dir_clean);
	pclose(tor_file);
	return command;
}

