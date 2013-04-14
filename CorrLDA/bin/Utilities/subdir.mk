################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Utilities/CDirichletOptimizer.cpp \
../Utilities/CUtilities.cpp \
../Utilities/cokus.cpp 

OBJS += \
./Utilities/CDirichletOptimizer.o \
./Utilities/CUtilities.o \
./Utilities/cokus.o 

CPP_DEPS += \
./Utilities/CDirichletOptimizer.d \
./Utilities/CUtilities.d \
./Utilities/cokus.d 


# Each subdirectory must supply rules for building sources it contributes
Utilities/%.o: ../Utilities/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


