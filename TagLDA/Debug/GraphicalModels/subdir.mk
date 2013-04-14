################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../GraphicalModels/CTagLDA.cpp 

OBJS += \
./GraphicalModels/CTagLDA.o 

CPP_DEPS += \
./GraphicalModels/CTagLDA.d 


# Each subdirectory must supply rules for building sources it contributes
GraphicalModels/%.o: ../GraphicalModels/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


