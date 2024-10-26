#include <opencv2/opencv.hpp>
#include <X11/Xlib.h>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <opencv2/freetype.hpp>
#include <opencv2/imgproc.hpp>

// Функция для инициализации камеры
std::unique_ptr<cv::VideoCapture> initializeCamera(int sensor_id, int width, int height, int framerate)
{
    std::string pipeline = "nvarguscamerasrc sensor-id=" + std::to_string(sensor_id) +
                           " ! video/x-raw(memory:NVMM),width=" + std::to_string(width) +
                           ",height=" + std::to_string(height) + ",framerate=" + std::to_string(framerate) + "/1" +
                           " ! nvvidconv ! video/x-raw,format=BGRx ! videoconvert ! video/x-raw,format=BGR ! appsink";

    // Попытка открыть видеопоток
    auto cap = std::make_unique<cv::VideoCapture>(pipeline, cv::CAP_GSTREAMER);
    if (!cap->isOpened())
    {
        throw std::runtime_error("Ошибка: не удалось открыть видеопоток с камеры " + std::to_string(sensor_id));
    }
    return cap;
}

int main()
{
    // Вычисляем ширину каждого окна
    Display *disp = XOpenDisplay(NULL);
    Screen *scrn = DefaultScreenOfDisplay(disp);
    int screen_width = scrn->width;
    int screen_height = scrn->height;
    int window_width = screen_width / 3;
    int window_height = screen_height;

    // Параметры текста для вывода на  экран
    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    double fontScale = 1.0; // Масштаб шрифта
    auto fontHeight = 20;
    auto line_type = cv::LINE_AA;
    auto bottomLeftOrigin = false;
    int thickness = 2;               // Толщина линий шрифта
    cv::Scalar color(255, 255, 255); // Цвет текста (белый)
    int baseline = 0;
    cv::Ptr<cv::freetype::FreeType2> ft2;
    ft2 = cv::freetype::createFreeType2();
    ft2->loadFontData("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 0);
    // Проверка успешности загрузки шрифта
    if (ft2.empty())
    {
        std::cerr << "Ошибка: не удалось загрузить шрифт!" << std::endl
                  << std::flush;
        return -1;
    }
    else
    {
        std::cerr << "Загрузка шрифта ОК!" << std::endl
                  << std::flush;
    }

    // камера 0 переменные
    int width_Frame0;    // Ширина кадра
    int height_Frame0;   // Высота кадра
    int channels_Frame0; // количество каналов

    // камера 1 переменные
    int width_Frame1;    // Ширина кадра
    int height_Frame1;   // Высота кадра
    int channels_Frame1; // количество каналов

    // Создание и настройка окон
    cv::namedWindow("Camera0", cv::WINDOW_NORMAL);
    cv::namedWindow("Camera1", cv::WINDOW_NORMAL);
    cv::namedWindow("Параметры", cv::WINDOW_NORMAL);

    // Размещение окон
    cv::resizeWindow("Camera0", window_width, window_height);
    cv::resizeWindow("Camera1", window_width, window_height);
    cv::resizeWindow("Параметры", window_width, window_height);

    // Перемещение окон
    cv::moveWindow("Camera0", 0, 0);                  // Окно 0 на левую часть экрана
    cv::moveWindow("Camera1", window_width, 0);       // Окно 1 в центр
    cv::moveWindow("Параметры", window_width * 2, 0); // Окно 2 на правую часть экрана

    // Добавление задержки, чтобы убедиться, что окна созданы с правильными параметрами
    cv::waitKey(500);

    std::cout << "Разрешение экрана: " << screen_width << "x" << screen_height << std::endl
              << std::flush;

    try
    {
        // Инициализация камер с использованием unique_ptr для автоматического освобождения ресурсов
        auto cap0 = initializeCamera(0, 1920, 1080, 60);
        auto cap1 = initializeCamera(1, 1920, 1080, 60);

        cv::Mat frame0, frame1;

        while (true)
        {
            // Захват кадров с камер
            *cap0 >> frame0;
            *cap1 >> frame1;

            // Проверка на пустые кадры
            if (frame0.empty())
            {
                throw std::runtime_error("Ошибка: Пустой кадр камеры №0!");
            }

            // Проверка на пустые кадры
            if (frame1.empty())
            {
                throw std::runtime_error("Ошибка: Пустой кадр камеры №1!");
            }

            // определяем размеры захваченных кадров камеры 0
            width_Frame0 = frame0.cols;  // Ширина кадра
            height_Frame0 = frame0.rows; // Высота кадра
            channels_Frame0 = frame0.channels();

            // определяем размеры захваченных кадров камеры 1
            width_Frame1 = frame1.cols;  // Ширина кадра
            height_Frame1 = frame1.rows; // Высота кадра
            channels_Frame1 = frame1.channels();

            // Создаем текст с информацией о размере кадра для камеры 0
            std::string text0 = "Размер: " + std::to_string(width_Frame0) + "x" + std::to_string(height_Frame0) +
                                ", Каналы: " + std::to_string(channels_Frame0);

            // Создаем текст с информацией о размере кадра для камеры 1
            std::string text1 = "Размер: " + std::to_string(width_Frame1) + "x" + std::to_string(height_Frame1) +
                                ", Каналы: " + std::to_string(channels_Frame1);

            // Определяем размеры текста для правильного позиционирования (для камеры 0)
            cv::Size textSize0 = cv::getTextSize(text0, fontFace, fontScale, thickness, &baseline);
            cv::Point textOrg0(10, textSize0.height + 10); // Позиция текста на изображении

            // Аналогично для камеры 1
            cv::Size textSize1 = cv::getTextSize(text1, fontFace, fontScale, thickness, &baseline);
            cv::Point textOrg1(10, textSize1.height + 10);

            // Накладываем текст на кадр камеры 0
            ft2->putText(frame0, text0, textOrg0, fontHeight, color, thickness, line_type, bottomLeftOrigin);

            // Накладываем текст на кадр камеры 1
            ft2->putText(frame1, text1, textOrg1, fontHeight, color, thickness, line_type, bottomLeftOrigin);

            // Отображение кадров
            cv::imshow("Camera0", frame0);
            cv::imshow("Camera1", frame1);

            // Выход при нажатии любой клавиши
            if (cv::waitKey(30) >= 0)
                break;
        }

        // Автоматическое освобождение ресурсов при выходе из try-блока
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        // // Освобождение ресурсов
        // if (cap0.isOpened())
        // {
        //     cap0.release();
        // }
        // if (cap1.isOpened())
        // {
        //     cap1.release();
        // }
        // Освобождение ресурсов и завершение программы
    }

    cv::destroyAllWindows(); // Закрытие всех окон
    return 0;
}
