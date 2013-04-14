################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/CCorrLDAPredictCPPUnit.cpp \
../src/CCorrLDATestCPPUnit.cpp \
../src/CCorrLDATrainCPPUnit.cpp \
../src/CorrLDA.cpp 

OBJS += \
./src/CCorrLDAPredictCPPUnit.o \
./src/CCorrLDATestCPPUnit.o \
./src/CCorrLDATrainCPPUnit.o \
./src/CorrLDA.o 

CPP_DEPS += \
./src/CCorrLDAPredictCPPUnit.d \
./src/CCorrLDATestCPPUnit.d \
./src/CCorrLDATrainCPPUnit.d \
./src/CorrLDA.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


