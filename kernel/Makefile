.PHONY: clean

CXXFLAGS = -g -std=c++20 -Iinclude -fno-rtti -fno-exceptions
LDFLAGS = -Tkernel.ld -g
SOURCES = $(wildcard kernel.cpp \
			 mem/*.cpp \
			 util/*.cpp \
			 exception/*.cpp \
			 process/*.cpp \
			 driver/*.cpp \
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
