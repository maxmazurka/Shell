TARGET = kubsh
VERSION = 0.1.0

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -D_FILE_OFFSET_BITS=64 -I/usr/include/fuse3 -static -no-pie
LDFLAGS = -lfuse3 -pthread -static

SOURCES = main.cpp vfs.cpp
OBJECTS = $(SOURCES:.cpp=.o)

.PHONY: all build run clean package test help

all: build

build: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: build
	./$(TARGET)

package: build
	@echo "📦 Создаём DEB-пакет..."
	mkdir -p package/usr/bin
	mkdir -p package/DEBIAN
	cp $(TARGET) package/usr/bin/
	chmod 755 package/usr/bin/$(TARGET)
	
	# Копируем postinst скрипт
	if [ -f debian/postinst ]; then \
		cp debian/postinst package/DEBIAN/; \
		chmod 755 package/DEBIAN/postinst; \
		echo "✓ postinst скрипт добавлен"; \
	fi
	
	@echo "Package: kubsh" > package/DEBIAN/control
	@echo "Version: $(VERSION)" >> package/DEBIAN/control
	@echo "Section: utils" >> package/DEBIAN/control
	@echo "Priority: optional" >> package/DEBIAN/control
	@echo "Architecture: amd64" >> package/DEBIAN/control
	@echo "Maintainer: Student <student@example.com>" >> package/DEBIAN/control
	@echo "Depends: libfuse3-4, fuse3, adduser" >> package/DEBIAN/control
	@echo "Description: Custom shell with VFS" >> package/DEBIAN/control
	@echo " Shell with command history, PATH execution, SIGHUP and VFS." >> package/DEBIAN/control
	dpkg-deb --build package kubsh_$(VERSION)_amd64.deb
	rm -rf package
	@echo "✅ Пакет создан: kubsh_$(VERSION)_amd64.deb"


test: build
	@echo "🧪 Запускаем тесты в Docker..."
	docker run --rm --privileged -v /dev/fuse:/dev/fuse -v $$(pwd):/mnt tyvik/kubsh_test:master bash -c "apt-get update && apt-get install -y libfuse3-4 fuse3 pkg-config && echo 'user_allow_other' >> /etc/fuse.conf && ldconfig && export LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu:$$LD_LIBRARY_PATH && cp /mnt/kubsh /usr/local/bin/ && chmod +x /usr/local/bin/kubsh && cd /opt && pytest -v"

clean:
	rm -f $(TARGET) *.o *.deb
	rm -rf package
	rm -rf /opt/users 2>/dev/null || true

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
