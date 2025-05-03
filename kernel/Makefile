.PHONY: clean

CXX = clang
LD = ld.lld
OBJCOPY = llvm-objcopy
CXXFLAGS = -g -std=c++20 -Iinclude -stdlib=libc++ --target=loongarch64-unknown-linux -fno-rtti -fno-exceptions
LDFLAGS = -Tkernel.ld -g
SOURCES = $(wildcard uart/*.cpp \
		     mem/*.cpp \
		     kernel.cpp \
		     util/*.cpp \
		     exception/*.cpp \
		     timer/*.cpp \
		     process/*.cpp \
		     sdcard/*.cpp \
	   )

OBJS = loader.o exception/context.o $(SOURCES:%.cpp=%.o)

kernel: $(OBJS)
	$(LD) $^ -o kernel.elf $(LDFLAGS)
	$(OBJCOPY) kernel.elf -O binary kernel.bin

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.s
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	find . -name \*.o -exec rm {} \;
	if [ -f kernel ]; then rm kernel; fi
	if [ -f kernel.elf ]; then rm kernel.elf; fi
