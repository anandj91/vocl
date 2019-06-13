################################################################################
#
# Copyright 1993-2015 NVIDIA Corporation.  All rights reserved.
#
# NOTICE TO USER:
#
# This source code is subject to NVIDIA ownership rights under U.S. and
# international Copyright laws.
#
# NVIDIA MAKES NO REPRESENTATION ABOUT THE SUITABILITY OF THIS SOURCE
# CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR
# IMPLIED WARRANTY OF ANY KIND.  NVIDIA DISCLAIMS ALL WARRANTIES WITH
# REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
# IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL,
# OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
# OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
# OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE
# OR PERFORMANCE OF THIS SOURCE CODE.
#
# U.S. Government End Users.  This source code is a "commercial item" as
# that term is defined at 48 C.F.R. 2.101 (OCT 1995), consisting  of
# "commercial computer software" and "commercial computer software
# documentation" as such terms are used in 48 C.F.R. 12.212 (SEPT 1995)
# and is provided to the U.S. Government only as a commercial end item.
# Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through
# 227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the
# source code with only those rights set forth herein.
#
################################################################################
#
# Makefile project only supported on Mac OS X and Linux Platforms)
#
################################################################################

CCFLAGS   := -fPIC
LDFLAGS   := 

INCLUDES  :=
INCLUDES  += -I/usr/local/cuda/include/ -I./include/

LIBRARIES := 
LIBRARIES += -ldl

SRC_DIR := src
INC_DIR := include
BUILD_DIR := build
LIB_DIR   := lib

# Target rules
all: build

build: $(LIB_DIR)/libvocl.so

$(LIB_DIR)/libvocl.so: $(BUILD_DIR)/vocl.o
	mkdir -p $(LIB_DIR)
	$(CXX) -shared $(LDFLAGS) -o $@ $+ $(LIBRARIES)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp ${INC_DIR}/%.h
	mkdir -p $(BUILD_DIR)
	$(CXX) $(INCLUDES) $(CCFLAGS) -o $@ -c $<

.PHONY: test
test: build
	$(CXX) $(INCLUDES) -lOpenCL -L/usr/local/cuda/lib64 -o test.out test/test.cpp
	$(CXX) $(INCLUDES) -lOpenCL -L/usr/local/cuda/lib64 -o test_dist.out test/test_dist.cpp

.PHONY: clean
clean:
	rm -rf $(LIB_DIR) $(BUILD_DIR)
