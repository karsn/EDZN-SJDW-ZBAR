SHELL = /bin/bash  
  
AllDirs := $(shell ls -R | grep '^\./.*:$$' | awk '{gsub(":","");print}') .  
Sources := $(foreach n,$(AllDirs) , $(wildcard $(n)/*.cpp))  
Objs := $(patsubst %.cpp,%.o, $(Sources))  
Deps := $(patsubst %.cpp,%.d, $(Sources))  
#StaticLib := libisli.a  
#DynamicLib := libisli.so  
Bin := test_detect  
  
#AllLibs : $(DynamicLib)  
#AllLibs : $(StaticLib)   
AllLibs : $(Bin)  

ifeq ($(ARCH),arm)   

CC := arm-linux-gnueabihf-gcc
CXX := arm-linux-gnueabihf-g++
AR := arm-linux-gnueabihf-ar
SYSROOT := /home/wangsh/ISLI_Scanner/rootfs-arm/
CXXFLAGS := -g -O2 -fPIC -Wall -std=c++11
CPPFLAGS := $(foreach n,$(AllDirs) , -I$(n))  
LDFLAGS := -lstdc++  

else

CC:=gcc 
CXX:=g++
AR:=ar
SYSROOT := /
CXXFLAGS := -g -O2 -fPIC -Wall -std=c++11
CPPFLAGS := $(foreach n,$(AllDirs) , -I$(n))  
LDFLAGS := -lstdc++

endif
	
$(StaticLib) : $(Objs)  
	$(AR) rcs $@ $^
	mkdir out
	cp $(StaticLib) out/
	cp ISLIIconDecoder.h out/
	cp ImageType.h out/
  
$(DynamicLib) : $(Objs)  
	$(CXX) -shared -o $@ $^ $(LDFLAGS)  
  
$(Bin) : $(Objs)  
	$(CXX) $(Objs) -o $@  
  
%.d : %.cpp  
	$(CC) -MT"$(<:.cpp=.o) $@" -MM $(CXXFLAGS) $(CPPFLAGS) $< > $@  
  
sinclude $(Deps)  

.PHONY : install
install:
	cp $(StaticLib) $(SYSROOT)/usr/lib/
#	if [ ! -d "$(SYSROOT)/usr/include/isli/"]; then mkdir $(SYSROOT)/usr/include/isli/; fi
	cp out/*.h $(SYSROOT)/usr/include/isli/
	  
.PHONY : clean  
clean:   
	rm -f $(Objs) $(Deps) $(StaticLib) $(DynamicLib) $(Bin)
	rm -rf out