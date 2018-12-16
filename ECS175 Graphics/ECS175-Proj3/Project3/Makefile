# Tyler Welsh, 912341695, ECS 175 Project 1
EXE = out

SRC_DIR = src
OBJ_DIR = obj
INC_DIR = ../include

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(addprefix $(OBJ_DIR)/, $(notdir $(SRC)))

CPPFLAGS += -Iinclude
CFLAGS += -c -Wall -g -lglut -lGL
LDFLAGS += -g -lglut -lGL

.PHONY all clean

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@
		
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ)
