#include <gst/gst.h>
#include <glib.h>
#include <iostream>

// Функция обратного вызова для обработки сообщений из шины GStreamer
static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data)
{
    GMainLoop *loop = (GMainLoop *)data;

    switch (GST_MESSAGE_TYPE(msg))
    {
    case GST_MESSAGE_EOS:
        g_print("Thread end\n");
        g_main_loop_quit(loop);
        break;

    case GST_MESSAGE_ERROR:
    {
        gchar *debug;
        GError *error;

        gst_message_parse_error(msg, &error, &debug);
        g_free(debug);

        g_printerr("Error: %s\n", error->message);
        g_error_free(error);

        g_main_loop_quit(loop);
        break;
    }
    default:
        break;
    }

    return TRUE;
}

int main(int argc, char *argv[])
{
    GstElement *pipeline, *source0, *source1, *nvvidconv0, *nvvidconv1, *queue0, *queue1, *nvvidconv2, *nvvidconv3, *compositor, *sink;
    GstCaps *caps_nv12, *caps_rgba;
    GstPad *sinkpad0, *sinkpad1;
    GstBus *bus;
    GMainLoop *loop;

    /* Инициализация GStreamer */
    gst_init(&argc, &argv);

    /* Создание основного цикла */
    loop = g_main_loop_new(NULL, FALSE);

    /* Создание элементов пайплайна */
    pipeline = gst_pipeline_new("camera-pipeline");

    source0 = gst_element_factory_make("nvarguscamerasrc", "source0");
    source1 = gst_element_factory_make("nvarguscamerasrc", "source1");

    nvvidconv0 = gst_element_factory_make("nvvidconv", "nvvidconv0");
    nvvidconv1 = gst_element_factory_make("nvvidconv", "nvvidconv1");

    queue0 = gst_element_factory_make("queue", "queue0");
    queue1 = gst_element_factory_make("queue", "queue1");

    nvvidconv2 = gst_element_factory_make("nvvidconv", "nvvidconv2"); // Конвертер для RGBA для камеры 0
    nvvidconv3 = gst_element_factory_make("nvvidconv", "nvvidconv3"); // Конвертер для RGBA для камеры 1

    compositor = gst_element_factory_make("nvcompositor", "compositor");

    sink = gst_element_factory_make("nvoverlaysink", "sink");

    if (!pipeline || !source0 || !source1 || !nvvidconv0 || !nvvidconv1 || !queue0 || !queue1 || !nvvidconv2 || !nvvidconv3 || !compositor || !sink)
    {
        g_printerr("Elements not created\n");
        return -1;
    }

    /* Установка свойств для источников камеры */
    g_object_set(G_OBJECT(source0), "sensor-id", 0, NULL);
    g_object_set(G_OBJECT(source1), "sensor-id", 1, NULL);

    /* Установка разрешения и частоты кадров для NV12 */
    caps_nv12 = gst_caps_from_string("video/x-raw(memory:NVMM), width=1920, height=1080, format=NV12, framerate=60/1");
    caps_rgba = gst_caps_from_string("video/x-raw, format=RGBA");

    /* Добавление элементов в пайплайн */
    gst_bin_add_many(GST_BIN(pipeline), source0, nvvidconv0, queue0, nvvidconv2, source1, nvvidconv1, queue1, nvvidconv3, compositor, sink, NULL);

    /* Связывание источника 0 */
    if (!gst_element_link(source0, nvvidconv0))
    {
        g_printerr("source0 with nvvidconv0 no bind\n");
        return -1;
    }


    // if (!gst_element_link_filtered(source0, nvvidconv0, caps_nv12))
    // {
    //     g_printerr("source0 with nvvidconv0 no bind\n");
    //     return -1;
    // }
    if (!gst_element_link(nvvidconv0, queue0))
    {
        g_printerr("nvvidconv0 with queue0 no bind\n");
        return -1;
    }
    if (!gst_element_link_filtered(queue0, nvvidconv2, caps_rgba))
    {
        g_printerr("queue0 with nvvidconv2 no bind\n");
        return -1;
    }

    /* Связывание источника 1 */
    if (!gst_element_link(source1, nvvidconv1))
    {
        g_printerr("source0 with nvvidconv0 no bind\n");
        return -1;
    }
    // if (!gst_element_link_filtered(source1, nvvidconv1, caps_nv12))
    // {
    //     g_printerr("source1 with nvvidconv1 no bind\n");
    //     return -1;
    // }
    if (!gst_element_link(nvvidconv1, queue1))
    {
        g_printerr("nvvidconv1 with queue1 no bind\n");
        return -1;
    }
    if (!gst_element_link_filtered(queue1, nvvidconv3, caps_rgba))
    {
        g_printerr("queue1 with nvvidconv3 no bind\n");
        return -1;
    }

    /* Связывание конвертеров RGBA с композитором */
    if (!gst_element_link(nvvidconv2, compositor))
    {
        g_printerr("nvvidconv2 with compositor no bind\n");
        return -1;
    }
    if (!gst_element_link(nvvidconv3, compositor))
    {
        g_printerr("nvvidconv3 with compositor no bind\n");
        return -1;
    }

    /* Связывание композитора с выходом */
    if (!gst_element_link(compositor, sink))
    {
        g_printerr("compositor with output no bind\n");
        return -1;
    }

    /* Настройка входных площадок композитора для позиционирования видеопотоков */
    sinkpad0 = gst_element_get_request_pad(compositor, "sink_%u");
    sinkpad1 = gst_element_get_request_pad(compositor, "sink_%u");

    if (!sinkpad0 || !sinkpad1)
    {
        g_printerr("Не удалось получить входные площадки композитора\n");
        return -1;
    }

    /* Конфигурация первого видеопотока (Камера 0) */
    g_object_set(G_OBJECT(sinkpad0), "xpos", 0, "ypos", 0, "width", 960, "height", 540, NULL);

    /* Конфигурация второго видеопотока (Камера 1) */
    g_object_set(G_OBJECT(sinkpad1), "xpos", 960, "ypos", 0, "width", 960, "height", 540, NULL);

    gst_object_unref(sinkpad0);
    gst_object_unref(sinkpad1);

    /* Добавление наблюдателя за шиной */
    bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_add_watch(bus, bus_call, loop);
    gst_object_unref(bus);

    /* Запуск воспроизведения */
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    /* Запуск цикла */
    g_main_loop_run(loop);

    /* Очистка ресурсов */
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    gst_caps_unref(caps_nv12);
    gst_caps_unref(caps_rgba);
    g_main_loop_unref(loop);

    return 0;
}
