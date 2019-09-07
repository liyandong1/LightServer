
AR := ar
ARFLAGS := crs

CC := g++ -g -o0 -std=c++11
LFLAGS := 
CFLAGS := -I$(DIR_INC) -I$(DIR_COMMON_INC) -pthread 

ifeq ($(DEBUG),true)
CFLAGS += -g
endif

MKDIR := mkdir
RM := rm -fr

