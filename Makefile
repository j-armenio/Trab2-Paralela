# Compilador
CC = mpicc

# Flags de compilação
CFLAGS = -O3 -Wall -Ilibs -fopenmp
LDFLAGS = -lm -fopenmp

# Diretórios
LIB_DIR = libs
OBJ_DIR = obj

# Arquivos fonte das bibliotecas (caminho relativo)
LIB_SRCS = $(LIB_DIR)/knn.c $(LIB_DIR)/max-heap.c $(LIB_DIR)/aux.c

# Arquivo principal
MAIN_SRC = main.c

# Gera a lista de objetos em obj/ mantendo o mesmo nome base
LIB_OBJS = $(patsubst $(LIB_DIR)/%.c,$(OBJ_DIR)/%.o,$(LIB_SRCS))
MAIN_OBJ = $(OBJ_DIR)/main.o

# Executável
TARGET = knn_mpi_omp

# regra padrão
all: $(TARGET)

# Linkagem final
$(TARGET): $(MAIN_OBJ) $(LIB_OBJS) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Regra genérica para compilar .c em obj/*.o
# aceita fontes em subpastas (libs/) ou na raiz
$(OBJ_DIR)/%.o: $(LIB_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# garante que o diretório obj exista
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Limpeza de objetos e executável
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Rebuild completo
rebuild: clean all

.PHONY: all clean rebuild
