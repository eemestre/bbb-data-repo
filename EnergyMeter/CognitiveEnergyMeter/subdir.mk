# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
main.cpp 

C_SRCS += \
Board.c \
AnalogCore.c \
CPT.c \
MAF.c \
DAQ.c \
Goertzel.c \
Mem.c \
MetrologyCore.c \
Waveform.c \
pru.c

OBJS += \
./Debug/Board.o \
./Debug/AnalogCore.o \
./Debug/CPT.o \
./Debug/MAF.o \
./Debug/DAQ.o \
./Debug/Goertzel.o \
./Debug/Mem.o \
./Debug/MetrologyCore.o \
./Debug/Waveform.o \
./Debug/main.o \
./Debug/pru.o

C_DEPS += \
./Debug/Board.d \
./Debug/AnalogCore.d \
./Debug/CPT.d \
./Debug/MAF.d \
./Debug/DAQ.d \
./Debug/Goertzel.d \
./Debug/Mem.d \
./Debug/MetrologyCore.d \
./Debug/Waveform.d \
./Debug/pru.d

CPP_DEPS += \
./Debug/main.d


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	${BBB}gcc -I system/am335x/am335x_pru_package/pru_sw/app_loader/include -L system/lib -O3 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	${BBB}g++ -O3 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


