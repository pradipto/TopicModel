################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../EMAlgorithms/CCorrLDAEMFunctionoid.cpp \
../EMAlgorithms/CModelEMFunctionoid.cpp 

OBJS += \
./EMAlgorithms/CCorrLDAEMFunctionoid.o \
./EMAlgorithms/CModelEMFunctionoid.o 

CPP_DEPS += \
./EMAlgorithms/CCorrLDAEMFunctionoid.d \
./EMAlgorithms/CModelEMFunctionoid.d 


# Each subdirectory must supply rules for building sources it contributes
EMAlgorithms/%.o: ../EMAlgorithms/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


