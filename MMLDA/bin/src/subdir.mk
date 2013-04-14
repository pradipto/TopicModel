################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/CMMLDAPredictCPPUnit.cpp \
../src/CMMLDATestCPPUnit.cpp \
../src/CMMLDATrainCPPUnit.cpp \
../src/MMLDA.cpp 

OBJS += \
./src/CMMLDAPredictCPPUnit.o \
./src/CMMLDATestCPPUnit.o \
./src/CMMLDATrainCPPUnit.o \
./src/MMLDA.o 

CPP_DEPS += \
./src/CMMLDAPredictCPPUnit.d \
./src/CMMLDATestCPPUnit.d \
./src/CMMLDATrainCPPUnit.d \
./src/MMLDA.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


