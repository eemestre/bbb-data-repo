-include makefile.init

BBB=./compiler/gcc-linaro-4.9-2016.02-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
endif

-include makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: CognitiveEnergyMeter

# Tool invocations
CognitiveEnergyMeter: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross G++ Linker'
	${BBB}g++ -L system/3.8.13-bone80/linux-3.8.13-bone80/include -L system/lib -L system/am335x/am335x_pru_package/pru_sw/app_loader/lib -L system/3.8.13-bone80/linux-3.8.13-bone80/lib,-Wl,-rpath-link -o "./Debug/CognitiveEnergyMeter" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(C++_DEPS)$(OBJS)$(C_DEPS)$(CC_DEPS)$(CPP_DEPS)$(EXECUTABLES)$(CXX_DEPS)$(C_UPPER_DEPS) ./Debug/CognitiveEnergyMeter
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include makefile.targets
