# Makefile для kubsh

# Имя программы
TARGET = kubsh
VERSION = 0.1.0

# Компилятор и флаги
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

# Исходные файлы (все .cpp в текущей папке)
SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

# Цели не-файлы
.PHONY: all build run clean package test help

# ============================================
# 1. Компиляция из исходников
# ============================================
all: build

build: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^

# ============================================
# 2. Запуск kubsh
# ============================================
run: build
	./$(TARGET)

# ============================================
# 3. Сборка deb-пакета
# ============================================
package: build
	@echo "📦 Создаём DEB-пакет..."
	# Структура пакета
	mkdir -p package/usr/bin
	mkdir -p package/DEBIAN
	# Копируем бинарник
	cp $(TARGET) package/usr/bin/
	chmod 755 package/usr/bin/$(TARGET)
	# Control файл
	@echo "Package: kubsh" > package/DEBIAN/control
	@echo "Version: $(VERSION)" >> package/DEBIAN/control
	@echo "Section: utils" >> package/DEBIAN/control
	@echo "Priority: optional" >> package/DEBIAN/control
	@echo "Architecture: amd64" >> package/DEBIAN/control
	@echo "Maintainer: Student <student@example.com>" >> package/DEBIAN/control
	@echo "Description: Custom shell implementation" >> package/DEBIAN/control
	@echo " Shell with command history, VFS and FUSE support." >> package/DEBIAN/control
	# Сборка
	dpkg-deb --build package kubsh_$(VERSION)_amd64.deb
	rm -rf package
	@echo "✅ Пакет создан: kubsh_$(VERSION)_amd64.deb"
	@ls -lh *.deb

# ============================================
# Тесты в Docker
test: build
	@echo "🧪 Запускаем тесты в Docker..."
	docker run --rm \
		-v $$(pwd):/mnt \
		tyvik/kubsh_test:master \
		bash -c "cp /mnt/kubsh /usr/local/bin/ && chmod +x /usr/local/bin/kubsh && cd /opt && pytest -v"
# ============================================
# Очистка
# ============================================
clean:
	rm -f $(TARGET) *.o
	rm -f *.deb
	rm -rf package

# ============================================
# Справка
# ============================================
help:
	@echo "========================================"
	@echo "  KUBSH - Makefile"
	@echo "========================================"
	@echo "  make build   - компиляция проекта"
	@echo "  make run     - запуск шелла"
	@echo "  make package - сборка deb-пакета"
	@echo "  make test    - запуск тестов (Docker)"
	@echo "  make clean   - очистка"
	@echo "  make help    - эта справка"
	@echo "========================================"
