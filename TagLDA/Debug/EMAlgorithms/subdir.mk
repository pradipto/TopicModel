################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../EMAlgorithms/CModelEMFunctionoid.cpp \
../EMAlgorithms/CTagLDAEMFunctionoid.cpp \
../EMAlgorithms/dirihypparam.cpp 

OBJS += \
./EMAlgorithms/CModelEMFunctionoid.o \
./EMAlgorithms/CTagLDAEMFunctionoid.o \
./EMAlgorithms/dirihypparam.o 

CPP_DEPS += \
./EMAlgorithms/CModelEMFunctionoid.d \
./EMAlgorithms/CTagLDAEMFunctionoid.d \
./EMAlgorithms/dirihypparam.d 


# Each subdirectory must supply rules for building sources it contributes
EMAlgorithms/%.o: ../EMAlgorithms/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


