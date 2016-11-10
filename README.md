# tmon

tmon is a directory monitor based on inotify, that can be run as a daemon to watch one directory and check for new torrent files.
Once a torrent file is found, it is opened and sent to Transmission with a download directory based on the type of files it contains.
It is written in standard C.

## Installing

Just run:

```
make all
```
and (as a root):

```
make install 
```

## Usage

Is pretty straight-forward. If [directory] is an ABSOLUTE path for the directory to be watched:

Start the daemon:
```
tmon -d [directory]
```
Start in verbose mode:

```
tmon -v [directory]
```
To set the paths for the download directories, edit settings.h and recompile.

## Supported formats

At the moment, tmon supports:

Music: mp3, flac, ogg

Video: mkv, avi, mp4

All other files get sent to MAINDIR (see settings.h).
