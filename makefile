ALL = torrent_monitor.c string_utils.c settings.h 
LOCPATH = ./bin
INSTALLPATH = /usr/local
DEBFLAGS = -g -Wall

all:
	mkdir -p $(LOCPATH)
	gcc $(ALL) -o $(LOCPATH)/tmon
debug:
	mkdir -p $(LOCPATH)/debug
	gcc $(ALL) -o $(LOCPATH)/debug/tmon_debug $(DEBFLAGS)
asan:
	gcc $(ALL) -o $(LOCPATH)/debug/tmon_debug_asan $(DEBFLAGS) -fsanitize=address
clean:
	rm $(LOCPATH)/asino
	rm $(LOCPATH)/debug/tmon_debug
distclean:
	rm -r $(LOCPATH)
install:
	cp $(LOCPATH)/tmon $(INSTALLPATH)/bin/tmon
uninstall:
	rm $(INSTALLPATH)/bin/tmon



	
