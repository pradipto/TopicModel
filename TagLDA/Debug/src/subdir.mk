################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/CTagLDATestCPPUnit.cpp \
../src/CTagLDATrainCPPUnit.cpp \
../src/CUtilities.cpp \
../src/TagLDA.cpp \
../src/cokus.cpp 

OBJS += \
./src/CTagLDATestCPPUnit.o \
./src/CTagLDATrainCPPUnit.o \
./src/CUtilities.o \
./src/TagLDA.o \
./src/cokus.o 

CPP_DEPS += \
./src/CTagLDATestCPPUnit.d \
./src/CTagLDATrainCPPUnit.d \
./src/CUtilities.d \
./src/TagLDA.d \
./src/cokus.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


