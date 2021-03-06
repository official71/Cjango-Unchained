#Makefile for our library

PROG = manage
CC = g++
CPPFLAGS = -std=c++1z -Wall -pthread -MMD -MP -fPIC
CPPFLAGS += -I./app/ -I./lib/
OVERALLCPP = $(CPPFLAGS)
OVERALLCPP += -L./http_parser/ -lhttp_response -lhttp_request -ldl

#CPPFLAGS += -ferror-limit=5   # doesn't work on Linux
DEBUG ?= 1
CJANGO_DYNLOAD ?= 1
ifeq ($(DEBUG), 1)
    CPPFLAGS += -g -DDEBUG
else
    #CPPFLAGS=-DNDEBUG
endif

ifeq ($(CJANGO_DYNLOAD), 1)
    CPPFLAGS +=  -DCJANGO_DYNLOAD -I./lib/simplefilewatcher/include/FileWatcher
else
    #CPPFLAGS=-DNDEBUG
endif

# CPPFLAGS = -std=c++1z -Wall
SFCDIR=lib/simplefilewatcher/build/intermediate/Debug
# OBJS = app/app.o app/main.o routing/router.o
OBJS = app/app.o routing/router.o
TESTOBJS = app/app.o routing/router.o app/logger.o \
  $(SFCDIR)/FileWatcher.o \
  $(SFCDIR)/FileWatcherOSX.o \
  $(SFCDIR)/FileWatcherLinux.o

UNITTESTDIR=tests
UNITTESTS  = $(UNITTESTDIR)/test-Router.o $(UNITTESTDIR)/test-http-request-parser.o
UNITTESTEXE=$(UNITTESTDIR)/tests-exe

HTTPPARSEROBJ = http_parser/http_request_parser.o\
	http_parser/http_request_body_parser.o \
	http_parser/url_encoded_form_parser.o \
	http_parser/http_request_line.o \
	http_parser/http_stream_reader.o
LIB_HTTP_RESPONSE = http_parser/libhttp_response.so
LIB_HTTP_REQUEST = http_parser/libhttp_request.so
HTTPPARSERLIB = $(LIB_HTTP_RESPONSE) $(LIB_HTTP_REQUEST)
REQ_SESSION=http_parser/http_request.o http_parser/http_session.o
RESPONSEOBJ=http_parser/http_response.o

TESTOBJS += $(HTTPPARSEROBJ)
OBJS += $(HTTPPARSEROBJ)
# substitute .o suffix with .d
DEPENDS   = $(TESTOBJS:.o=.d)

# $@ : target file name  ( left side of ':')
# $^ : source file names (right side of ':')
# Note: $< is only the first source file

# -ldl is needed for Linux
$(PROG) : $(TESTOBJS) app/manage.cpp $(HTTPPARSERLIB)
	$(CC) -o $@ $^ $(OVERALLCPP)

# main.o : app/main.cpp
# 	$(CC) $(CPPFLAGS) -c $^
msocket.o : app/msocket.cpp
	$(CC) $(CPPFLAGS) -c $^
app.o : app/app.cpp
	$(CC) $(CPPFLAGS) -c $^
router.o : routing/router.cpp
	$(CC) $(CPPFLAGS) -c $^

#app/logger.o: app/logger.cpp
#	$(CC) $(CPPFLAGS) -fPIC -o $@ -c $^
http_parser/libhttp_response.so:  $(REQ_SESSION) $(RESPONSEOBJ) app/logger.o
	$(CC) $(CPPFLAGS) -shared -o $@ $^
http_parser/libhttp_request.so: $(REQ_SESSION) app/logger.o
	$(CC) $(CPPFLAGS) -shared -o $@ $^

# Causes g++: fatal error: cannot specify -o with -c, -S or -E with multiple files
# On ubuntu with g++ 6.2.0
#app/logger.o: app/logger.cpp
#	$(CC) $(CPPFLAGS) -fPIC -o $@ -c $^

lib/simplefilewatcher/build/intermediate/Debug/FileWatcher.o:
	make -C lib/simplefilewatcher/build/gmake
lib/simplefilewatcher/build/intermediate/Debug/FileWatcherOSX.o:
	make -C lib/simplefilewatcher/build/gmake
lib/simplefilewatcher/build/intermediate/Debug/FileWatcherLinux.o:
	make -C lib/simplefilewatcher/build/gmake

test: $(UNITTESTS) $(TESTOBJS) $(HTTPPARSERLIB)
	$(CC) $(CPPFLAGS) $(UNITTESTDIR)/dummy-callback.cpp -fPIC -c -o $(UNITTESTDIR)/dummy-callback.o
	$(CC) -shared -o $(UNITTESTDIR)/dummy-callback.so $(UNITTESTDIR)/dummy-callback.o $(OVERALLCPP)
	$(CC) $(UNITTESTDIR)/test-main.cpp -c -o $(UNITTESTDIR)/test-main.o $(OVERALLCPP)
	$(CC) $(UNITTESTDIR)/test-main.o $(UNITTESTS) $(TESTOBJS) -o $(UNITTESTEXE) $(OVERALLCPP)
test-run:
	LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:./http_parser $(UNITTESTEXE) -r compact
test-clean:
	rm -f $(UNITTESTS) $(UNITTESTDIR)/dummy-callback.*o $(UNITTESTDIR)/*.d $(UNITTESTDIR)/test-main.o $(UNITTESTEXE)

clean:
	rm -f app/*.o routing/*.o http_parser/*.o http_parser/*.so lib/simplefilewatcher/build/intermediate/Debug/*.o
	rm -f core $(PROG) $(OBJS) $(HTTPPARSEROBJ) $(HTTPPARSEROBJ:.o=.d) $(DEPENDS) $(UNITTESTS) $(UNITTESTEXE) $(REQ_SESSION) $(REQ_SESSION:.o=.d) $(RESPONSEOBJ:.o=.d) $(LIB_HTTP_REQUEST) $(LIB_HTTP_RESPONSE)


# By -MMD -MP flags, dependency between source files are written in *.d files
# These *.d files are included here in order to auto-recompile when header files change
-include $(DEPENDS)
