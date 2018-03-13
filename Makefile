ifeq ($(ARCH),arm)

CC=arm-linux-gnueabihf-gcc
CXX=arm-linux-gnueabihf-g++

SYSROOT=/home/wangsh/ISLI_Scanner/rootfs-arm/ 

#LIBS+= -Wl,-Bstatic -lisli -lzbar -Wl,-Bdynamic -lpng16 -lpthread
#INC+=-I/home/wangsh/ISLI_Scanner/decoder/zbar-install-arm/include
#LDFLAGS+=-L/usr/lib/arm-linux-gnueabihf/ -L/home/wangsh/ISLI_Scanner/decoder/zbar-install-arm/lib
#	LDFLAGS+=-L/home/wangsh/ISLI_Scanner/decoder/isli_icon/

else

CC=gcc
CXX=g++

SYSROOT=/

#LIBS+= -Wl,-Bstatic -lisli -Wl,-Bdynamic -lzbar -lpng16 -lpthread 
#INC+=-I/home/wangsh/ISLI_Scanner/decoder/zbar-install/include
#LDFLAGS+=-L/usr/local/lib/ -L/home/wangsh/ISLI_Scanner/decoder/zbar-install/lib
#LDFLAGS+=-L/home/wangsh/ISLI_Scanner/decoder/isli_icon/

endif

CFLAGS := -Wall -ggdb3
CXXFLAGS := -Wall -ggdb3 -std=c++11
PRJ_PATH = $(shell pwd)
ZBAR_INC = $(PRJ_PATH)/zbar/include/
ZBAR_LIB = $(PRJ_PATH)/zbar/build/zbar/.libs/


#LIBS+=-lzbar -lpng16 -lpthread
#ALL_PROGRAM = test_video_shm bar_srv isli_icon_srv isli_line_srv
ALL_PROGRAM = test_video_shm qr_srv

all: $(ALL_PROGRAM)

#test_video: test_video.c
#	$(CC) -o $@ $^ $(INC) $(LDFLAGS) $(CFLAGS) $(LIBS)

test_video_shm: test_video_shm.c decoder_srv.h
	$(CC) -o $@ $^ --sysroot=$(SYSROOT) $(CFLAGS) -lpng16 -lpthread -Dpng_set_gray_1_2_4_to_8=png_set_expand_gray_1_2_4_to_8

qr_srv: main.c decoder_srv.h
	$(CC) -o $@ $^ --sysroot=$(SYSROOT) $(CFLAGS) -I$(ZBAR_INC) -L$(ZBAR_LIB) -Wl,-Bstatic -lzbar -Wl,-Bdynamic -lpthread

isli_icon_srv : isli_icon_srv.cpp decoder_srv.h
	$(CXX) -o $@ $^ --sysroot=$(SYSROOT) $(CXXFLAGS) -static -lisli

isli_line_srv: isli_line_srv.cpp decoder_srv.h
	$(CXX) -o $@ $^ --sysroot=$(SYSROOT) $(CXXFLAGS) -static -lisli_line
		
.PHONY : clean		
clean:
	rm -f $(ALL_PROGRAM) *.o