cmake_minimum_required(VERSION 3.10)
project(LinuxOpenCV)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Поиск пакетов
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
find_package(OpenCV REQUIRED)
find_package(X11 REQUIRED)

# Вывод отладочных сообщений
message(STATUS "GTK3_INCLUDE_DIRS: ${GTK3_INCLUDE_DIRS}")
message(STATUS "GTK3_LIBRARIES: ${GTK3_LIBRARIES}")
message(STATUS "OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV_LIBS: ${OpenCV_LIBS}")
message(STATUS "X11_INCLUDE_DIR: ${X11_INCLUDE_DIR}")
message(STATUS "X11_LIBRARIES: ${X11_LIBRARIES}")

# Добавление исполняемого файла
add_executable(${PROJECT_NAME} main.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "myfile3")

# Указание директории для исполняемого файла
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Включение директорий заголовочных файлов для цели
target_include_directories(${PROJECT_NAME} PRIVATE ${GTK3_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS} ${X11_INCLUDE_DIR})

# Определение флагов компиляции для GTK3
target_compile_options(${PROJECT_NAME} PRIVATE ${GTK3_CFLAGS_OTHER})

# Линковка библиотек с использованием формы с ключевыми словами
target_link_libraries(${PROJECT_NAME} PRIVATE ${GTK3_LIBRARIES} ${OpenCV_LIBS} ${X11_LIBRARIES})
