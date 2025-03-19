.PHONY: clean

CXX = $(PREFIX)g++
OBJCOPY = $(PREFIX)objcopy
CXXFLAGS = -march=loongarch64 -mabi=lp64d -g -std=c++20 -fno-stack-protector -nostartfiles -Iinclude -fno-rtti -fno-exceptions
LDFLAGS = -Tkernel.ld -g -std=c++20 -fno-stack-protector -nostartfiles -fno-rtti -fno-exceptions
SOURCES = $(wildcard uart/*.cpp \
		     mem/*.cpp \
		     kernel.cpp \
		     util/*.cpp \
		     exception/*.cpp \
		     timer/*.cpp \
		     process/*.cpp \
		     sdcard/*.cpp \
		     fs/*.cpp \
	   )

OBJS = loader.o exception/context.o $(SOURCES:%.cpp=%.o)

kernel: $(OBJS)
	$(CXX) $^ -o kernel.elf $(LDFLAGS)
	$(OBJCOPY) kernel.elf -O binary kernel.bin

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.s
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	find . -name \*.o -exec rm {} \;
	if [ -f kernel ]; then rm kernel; fi
	if [ -f kernel.elf ]; then rm kernel.elf; fi
