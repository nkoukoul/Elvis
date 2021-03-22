IDIR = lib
CC = /usr/bin/g++
CFLAGS = -std=c++17 -g -I$(IDIR)

ODIR = obj

BINDIR = bin

LIBS = -lpthread -lpqxx -lpq -lcrypto++

_OBJ = main.o app_context.o controllers.o io_context.o \
			json_utils.o models.o request_context.o \
			response_context.o utils.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(BINDIR)/app: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

$(ODIR)/main.o : main.cc app_context.h controllers.h
		$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/app_context.o : app_context.cc app_context.h common_headers.h \
					event_queue.h io_context.h \
					request_context.h route_manager.h \
					json_utils.h response_context.h \
					cache.h db_connector.h utils.h \
					logger.h
		$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/controllers.o : controllers.cc controllers.h app_context.h \
					response_context.h models.h
		$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/io_context.o : io_context.cc io_context.h request_context.h \
					response_context.h app_context.h event_queue.h
		$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/json_utils.o : json_utils.cc json_utils.h
		$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/models.o : models.cc models.h app_context.h
		$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/request_context.o : request_context.cc request_context.h \
						io_context.h app_context.h event_queue.h
		$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/response_context.o : response_context.cc response_context.h \
						io_context.h app_context.h event_queue.h
		$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/utils.o : utils.cc utils.h
		$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	-rm $(BINDIR)/app $(ODIR)/*.o