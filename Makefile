CC = g++
CFLAGS = -Wall -Wextra -std=c++11 -Iinclude -g
LDFLAGS = -lboost_system -lboost_thread  # Добавлены библиотеки Boost

# остальная часть Makefile остается без изменений

# директории
SRC_DIR = ./src
BUILD_DIR = ./build
BIN_DIR = ./bin

# Исходные файлы
# wildcard ищет все файлы указанные файлы и возвращает их список, если файлов нет то список пуст
# SRCS_SERVER = $(wildcard $(SRC_DIR)/server/*.cpp)
# SRCS_CLIENT = $(wildcard $(SRC_DIR)/client/*.cpp)
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# Объектные файлы
# patsubst создаёт список на основе последнего аргумента, меняя расположение и расширения
# OBJS_CLIENT = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS_CLIENT))
# OBJS_SERVER = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS_SERVER))
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

# итоговые файлы
# TARGET_SERVER = $(BIN_DIR)/server
# TARGET_CLIENT = $(BIN_DIR)/client
TARGET = $(BIN_DIR)/test

# операция выполняемая по стандарту
all: create_dirs $(TARGET)

# Создание директорий
create_dirs:
	@mkdir -p $(BUILD_DIR)/client
	@mkdir -p $(BUILD_DIR)/server
	@mkdir -p $(BIN_DIR)

$(TARGET): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

# Сборка исполняемого файла сервера из объектных
$(TARGET_SERVER): $(OBJS_SERVER)
	$(CC) $^ -o $@ $(LDFLAGS)

# Сборка исполняемого файла клиента из объектных
$(TARGET_CLIENT): $(OBJS_CLIENT)
	$(CC) $^ -o $@ $(LDFLAGS)

# Правило для объектных файлов
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Очистка
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: all create_dirs clean server client