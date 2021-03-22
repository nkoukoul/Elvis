IDIR = elvis
CC = /usr/bin/g++
CFLAGS = -std=c++17 -g

SDIR = samples/example_app

ODIR = obj

BINDIR = bin

LIBS = -lpthread -lpqxx -lpq -lcrypto++

_OBJ = main.o my_controllers.o my_models.o app_context.o \
			controllers.o io_context.o json_utils.o \
			request_context.o \
			response_context.o utils.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(BINDIR)/app: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

$(ODIR)/main.o : $(SDIR)/main.cc $(SDIR)/my_controllers.h elvis/app_context.h
		$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/my_controllers.o : $(SDIR)/my_controllers.cc $(SDIR)/my_controllers.h \
							elvis/app_context.h elvis/controllers.h \
							$(SDIR)/my_models.h
		$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/my_models.o : $(SDIR)/my_models.cc $(SDIR)/my_models.h elvis/app_context.h
		$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/app_context.o : elvis/app_context.cc elvis/app_context.h elvis/common_headers.h \
					elvis/event_queue.h elvis/io_context.h \
					elvis/request_context.h elvis/route_manager.h \
					elvis/json_utils.h elvis/response_context.h \
					elvis/cache.h elvis/db_connector.h elvis/utils.h \
					elvis/logger.h
		$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/controllers.o : elvis/controllers.cc elvis/controllers.h elvis/app_context.h \
					elvis/response_context.h elvis/models.h
		$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/io_context.o : elvis/io_context.cc elvis/io_context.h elvis/request_context.h \
					elvis/response_context.h elvis/app_context.h elvis/event_queue.h
		$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/json_utils.o : elvis/json_utils.cc elvis/json_utils.h
		$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/request_context.o : elvis/request_context.cc elvis/request_context.h \
						elvis/io_context.h elvis/app_context.h elvis/event_queue.h
		$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/response_context.o : elvis/response_context.cc elvis/response_context.h \
						elvis/io_context.h elvis/app_context.h elvis/event_queue.h
		$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/utils.o : elvis/utils.cc elvis/utils.h
		$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	-rm $(BINDIR)/app $(ODIR)/*.o