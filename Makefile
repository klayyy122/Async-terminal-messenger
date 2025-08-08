CC = g++
CFLAGS = -Wall -Wextra -std=c++1 -Iinclude -g
LDFLAGS = -lboost_system -lpthread

# Директории
SRC_DIR = ./src
BUILD_DIR = ./build
BIN_DIR = ./bin

# Исходные файлы
SRCS_SERVER = $(wildcard $(SRC_DIR)/server/*.cpp)
SRCS_CLIENT = $(wildcard $(SRC_DIR)/client/*.cpp)

# Объектные файлы
OBJS_CLIENT = $(patsubst $(SRC_DIR)/client/%.cpp,$(BUILD_DIR)/client/%.o,$(SRCS_CLIENT))
OBJS_SERVER = $(patsubst $(SRC_DIR)/server/%.cpp,$(BUILD_DIR)/server/%.o,$(SRCS_SERVER))

# Итоговые файлы
TARGET_SERVER = $(BIN_DIR)/server
TARGET_CLIENT = $(BIN_DIR)/client

# Основная цель
all: create_dirs $(TARGET_CLIENT) $(TARGET_SERVER)

# Создание директорий
create_dirs:
	@mkdir -p $(BUILD_DIR)/client
	@mkdir -p $(BUILD_DIR)/server
	@mkdir -p $(BIN_DIR)

# Сборка сервера
$(TARGET_SERVER): $(OBJS_SERVER)
	$(CC) $^ -o $@ $(LDFLAGS)

# Сборка клиента
$(TARGET_CLIENT): $(OBJS_CLIENT)
	$(CC) $^ -o $@ $(LDFLAGS)

# Правило для объектных файлов сервера
$(BUILD_DIR)/server/%.o: $(SRC_DIR)/server/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Правило для объектных файлов клиента
$(BUILD_DIR)/client/%.o: $(SRC_DIR)/client/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Очистка
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# Псевдоцели
.PHONY: all create_dirs clean