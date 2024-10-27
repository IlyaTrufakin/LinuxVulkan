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
    GstElement *pipeline, *source0, *source1;
    GstElement *queue0, *queue1;
    GstElement *nvvidconv0, *nvvidconv1;
    GstElement *capsfilter0, *capsfilter1;
    GstElement *compositor, *sink;
    GstCaps *caps;
    GstBus *bus;
    GMainLoop *loop;

    gst_init(&argc, &argv);
    loop = g_main_loop_new(NULL, FALSE);
    pipeline = gst_pipeline_new("camera-pipeline");

    // Создание элементов
    source0 = gst_element_factory_make("nvarguscamerasrc", "source0");
    source1 = gst_element_factory_make("nvarguscamerasrc", "source1");
    queue0 = gst_element_factory_make("queue", "queue0");
    queue1 = gst_element_factory_make("queue", "queue1");
    nvvidconv0 = gst_element_factory_make("nvvideoconvert", "nvvidconv0");
    nvvidconv1 = gst_element_factory_make("nvvideoconvert", "nvvidconv1");
    capsfilter0 = gst_element_factory_make("capsfilter", "capsfilter0");
    capsfilter1 = gst_element_factory_make("capsfilter", "capsfilter1");
    compositor = gst_element_factory_make("nvcompositor", "compositor");
    sink = gst_element_factory_make("nvoverlaysink", "sink");

    if (!pipeline || !source0 || !source1 || !queue0 || !queue1 ||
        !nvvidconv0 || !nvvidconv1 || !capsfilter0 || !capsfilter1 ||
        !compositor || !sink)
    {
        g_printerr("Failed to create elements\n");
        return -1;
    }

    // Настройка источников камеры
    g_object_set(G_OBJECT(source0),
                 "sensor-id", 0,
                 "bufapi-version", 1,
                 NULL);
    g_object_set(G_OBJECT(source1),
                 "sensor-id", 1,
                 "bufapi-version", 1,
                 NULL);

    // Настройка конвертеров видео
    g_object_set(G_OBJECT(nvvidconv0),
                 "nvbuf-memory-type", 3,
                 NULL);
    g_object_set(G_OBJECT(nvvidconv1),
                 "nvbuf-memory-type", 3,
                 NULL);

    // Caps для выхода с камер
    GstCaps *src_caps = gst_caps_from_string(
        "video/x-raw(memory:NVMM), "
        "width=(int)1920, height=(int)1080, "
        "format=(string)NV12, framerate=(fraction)30/1");

    // Caps для входа в nvvideoconvert
    GstCaps *conv_caps = gst_caps_from_string(
        "video/x-raw(memory:NVMM), "
        "width=(int)960, height=(int)540, "
        "format=(string)NV12, framerate=(fraction)30/1");

    // Caps для входа в композитор
    GstCaps *comp_caps = gst_caps_from_string(
        "video/x-raw(memory:NVMM), "
        "width=(int)960, height=(int)540, "
        "format=(string)NV12, framerate=(fraction)30/1");

    // Caps для выхода композитора должны соответствовать его реальному выходному формату
    GstCaps *comp_output_caps = gst_caps_from_string(
        "video/x-raw(memory:NVMM), "
        "width=(int)1920, height=(int)1080, "
        "format=(string)NV12, framerate=(fraction)30/1");

    // GstCaps *comp_output_caps = gst_caps_from_string(
    //     "video/x-raw(memory:NVMM), "
    //     "width=(int)1920, height=(int)1080, "
    //     "format=(string)NV12, framerate=(fraction)30/1");

    // После создания caps
    if (!src_caps || !conv_caps || !comp_caps || !comp_output_caps)
    {
        g_printerr("Failed to create caps\n");
        return -1;
    }

    // Добавим отладочный вывод
    g_print("Source caps: %s\n", gst_caps_to_string(src_caps));
    g_print("Convert caps: %s\n", gst_caps_to_string(conv_caps));
    g_print("Compositor caps: %s\n", gst_caps_to_string(comp_caps));
    g_print("Output caps: %s\n", gst_caps_to_string(comp_output_caps));

    // Создаем дополнительные capsfilter'ы
    GstElement *src_capsfilter0 = gst_element_factory_make("capsfilter", "src_capsfilter0");
    GstElement *src_capsfilter1 = gst_element_factory_make("capsfilter", "src_capsfilter1");
    GstElement *conv_capsfilter0 = gst_element_factory_make("capsfilter", "conv_capsfilter0");
    GstElement *conv_capsfilter1 = gst_element_factory_make("capsfilter", "conv_capsfilter1");
    GstElement *comp_capsfilter0 = gst_element_factory_make("capsfilter", "comp_capsfilter0");
    GstElement *comp_capsfilter1 = gst_element_factory_make("capsfilter", "comp_capsfilter1");
    GstElement *comp_output_capsfilter = gst_element_factory_make("capsfilter", "comp_output_capsfilter");
    // Добавим поддержку nvvidconv для корректной работы с буферами
    GstElement *post_conv = gst_element_factory_make("nvvideoconvert", "post_conv");
    if (!post_conv)
    {
        g_printerr("Failed to create post converter\n");
        return -1;
    }

    // Создаем дополнительный конвертер
    GstElement *final_convert = gst_element_factory_make("nvvideoconvert", "final_convert");

    if (!final_convert)
    {
        g_printerr("Failed to create final converter\n");
        return -1;
    }

    if (!src_capsfilter0 || !src_capsfilter1 || !conv_capsfilter0 ||
        !conv_capsfilter1 || !comp_capsfilter0 || !comp_capsfilter1 || !comp_output_capsfilter)
    {
        g_printerr("Failed to create additional capsfilters\n");
        return -1;
    }

    g_object_set(G_OBJECT(comp_output_capsfilter), "caps", comp_output_caps, NULL);
    g_object_set(G_OBJECT(src_capsfilter0), "caps", src_caps, NULL);
    g_object_set(G_OBJECT(src_capsfilter1), "caps", src_caps, NULL);
    g_object_set(G_OBJECT(conv_capsfilter0), "caps", conv_caps, NULL);
    g_object_set(G_OBJECT(conv_capsfilter1), "caps", conv_caps, NULL);
    g_object_set(G_OBJECT(comp_capsfilter0), "caps", comp_caps, NULL);
    g_object_set(G_OBJECT(comp_capsfilter1), "caps", comp_caps, NULL);
    // Настраиваем конвертер
    //   g_object_set(G_OBJECT(final_convert), "nvbuf-memory-type", 3, NULL);
    // Изменим настройки nvvideoconvert
    g_object_set(G_OBJECT(final_convert), "nvbuf-memory-type", 4, NULL); // Изменим тип памяти с 3 на 4

    // Настройка nvvideoconvert
    g_object_set(G_OBJECT(nvvidconv0), "gpu-id", 0, "nvbuf-memory-type", 4, NULL); // NVBUF_MEM_SURFACE_ARRAY
    g_object_set(G_OBJECT(nvvidconv1), "gpu-id", 0, "nvbuf-memory-type", 4, NULL);
    g_object_set(G_OBJECT(post_conv), "gpu-id", 0, "nvbuf-memory-type", 4, NULL);

    // Настройка композитора
    g_object_set(G_OBJECT(compositor), "gpu-id", 0, "batch-size", 1, NULL);

    gst_caps_unref(src_caps);
    gst_caps_unref(conv_caps);
    gst_caps_unref(comp_caps);
    gst_caps_unref(comp_output_caps);

    // Вставляем здесь код проверки caps
    // Проверка поддерживаемых форматов

    GstPad *compositor_src_pad = gst_element_get_static_pad(compositor, "src");
    GstPad *capsfilter_sink_pad = gst_element_get_static_pad(comp_output_capsfilter, "sink");

    if (compositor_src_pad)
    {
        GstCaps *allowed_caps = gst_pad_get_allowed_caps(compositor_src_pad);
        if (allowed_caps)
        {
            g_print("Compositor src allowed caps: %s\n", gst_caps_to_string(allowed_caps));
            gst_caps_unref(allowed_caps);
        }
        else
        {
            g_print("No allowed caps for compositor src\n");
        }
        gst_object_unref(compositor_src_pad);
    }

    if (capsfilter_sink_pad)
    {
        GstCaps *allowed_caps = gst_pad_get_allowed_caps(capsfilter_sink_pad);
        if (allowed_caps)
        {
            g_print("Output capsfilter sink allowed caps: %s\n", gst_caps_to_string(allowed_caps));
            gst_caps_unref(allowed_caps);
        }
        else
        {
            g_print("No allowed caps for output capsfilter sink\n");
        }
        gst_object_unref(capsfilter_sink_pad);
    }

    //  Настройка sink
    // g_object_set(G_OBJECT(sink),
    //              "sync", FALSE,
    //              "async", FALSE,
    //              NULL);
    // Добавим дополнительные настройки для sink
    // g_object_set(G_OBJECT(sink), "sync", FALSE, "async", FALSE, "nvbuf-memory-type", 4, NULL);
    // Тот же тип памяти

    // Настройка sink
    g_object_set(G_OBJECT(sink), "sync", FALSE, "async", FALSE, NULL);

    // Caps для выхода композитора

    // Добавление всех элементов в pipeline
    // gst_bin_add_many(GST_BIN(pipeline),
    //                  source0, src_capsfilter0, queue0, nvvidconv0, conv_capsfilter0, comp_capsfilter0,
    //                  source1, src_capsfilter1, queue1, nvvidconv1, conv_capsfilter1, comp_capsfilter1,
    //                  compositor, comp_output_capsfilter, sink, NULL);
    // Добавляем в pipeline
    // gst_bin_add_many(GST_BIN(pipeline),
    //                  source0, src_capsfilter0, queue0, nvvidconv0, conv_capsfilter0, comp_capsfilter0,
    //                  source1, src_capsfilter1, queue1, nvvidconv1, conv_capsfilter1, comp_capsfilter1,
    //                  compositor, final_convert, comp_output_capsfilter, sink, NULL);

    gst_bin_add_many(GST_BIN(pipeline),
                     source0, src_capsfilter0, queue0, nvvidconv0, conv_capsfilter0, comp_capsfilter0,
                     source1, src_capsfilter1, queue1, nvvidconv1, conv_capsfilter1, comp_capsfilter1,
                     compositor, post_conv, comp_output_capsfilter, sink, NULL);

    // Связывание элементов первой камеры
    if (!gst_element_link_many(source0, src_capsfilter0, queue0, nvvidconv0, conv_capsfilter0, comp_capsfilter0, NULL))
    {
        g_printerr("Failed to link elements for camera 0\n");
        return -1;
    }

    // Связывание элементов второй камеры
    if (!gst_element_link_many(source1, src_capsfilter1, queue1, nvvidconv1, conv_capsfilter1, comp_capsfilter1, NULL))
    {
        g_printerr("Failed to link elements for camera 1\n");
        return -1;
    }

    // Получение pad'ов
    GstPad *compositor_sink_pad0 = gst_element_get_request_pad(compositor, "sink_%u");
    GstPad *compositor_sink_pad1 = gst_element_get_request_pad(compositor, "sink_%u");
    GstPad *comp_capsfilter0_src_pad = gst_element_get_static_pad(comp_capsfilter0, "src");
    GstPad *comp_capsfilter1_src_pad = gst_element_get_static_pad(comp_capsfilter1, "src");

    // Связывание pad'ов
    GstPadLinkReturn link_ret0 = gst_pad_link(comp_capsfilter0_src_pad, compositor_sink_pad0);
    GstPadLinkReturn link_ret1 = gst_pad_link(comp_capsfilter1_src_pad, compositor_sink_pad1);

    if (link_ret0 != GST_PAD_LINK_OK || link_ret1 != GST_PAD_LINK_OK)
    {
        g_printerr("Failed to link pads: pad0=%d, pad1=%d\n", link_ret0, link_ret1);
        g_printerr("Possible values: OK=%d, WRONG_HIERARCHY=%d, WRONG_DIRECTION=%d, "
                   "NOFORMAT=%d, NOSCHED=%d, REFUSED=%d\n",
                   GST_PAD_LINK_OK, GST_PAD_LINK_WRONG_HIERARCHY,
                   GST_PAD_LINK_WRONG_DIRECTION, GST_PAD_LINK_NOFORMAT,
                   GST_PAD_LINK_NOSCHED, GST_PAD_LINK_REFUSED);
        return -1;
    }

    // // Связывание pad'ов
    // if (gst_pad_link(comp_capsfilter0_src_pad, compositor_sink_pad0) != GST_PAD_LINK_OK ||
    //     gst_pad_link(comp_capsfilter1_src_pad, compositor_sink_pad1) != GST_PAD_LINK_OK)
    // {
    //     g_printerr("Failed to link pads\n");
    //     return -1;
    // }

    // Настройка свойств sink pad'ов
    g_object_set(compositor_sink_pad0,
                 "xpos", 0,
                 "ypos", 0,
                 "width", 960,
                 "height", 540,
                 NULL);
    g_object_set(compositor_sink_pad1,
                 "xpos", 960,
                 "ypos", 0,
                 "width", 960,
                 "height", 540,
                 NULL);

    // Освобождение pad'ов
    gst_object_unref(comp_capsfilter0_src_pad);
    gst_object_unref(comp_capsfilter1_src_pad);
    gst_object_unref(compositor_sink_pad0);
    gst_object_unref(compositor_sink_pad1);

    // Изменяем связывание композитора с sink
    // if (!gst_element_link_many(compositor, comp_output_capsfilter, sink, NULL))
    // {
    //     g_printerr("Failed to link compositor chain: compositor -> comp_output_capsfilter -> sink\n");
    //     return -1;
    // }

    // Перед попыткой связывания
    // Вывод информации о поддерживаемых форматах
    GstCaps *allowed_caps;
    allowed_caps = gst_pad_get_allowed_caps(gst_element_get_static_pad(compositor, "src"));
    g_print("Compositor src allowed caps: %s\n", gst_caps_to_string(allowed_caps));
    gst_caps_unref(allowed_caps);

    allowed_caps = gst_pad_get_allowed_caps(gst_element_get_static_pad(comp_output_capsfilter, "sink"));
    g_print("Output capsfilter sink allowed caps: %s\n", gst_caps_to_string(allowed_caps));
    gst_caps_unref(allowed_caps);

    // Изменяем связывание композитора с sink
    // if (!gst_element_link_many(compositor, final_convert, comp_output_capsfilter, sink, NULL))
    // {
    //     g_printerr("Failed to link compositor chain: compositor -> final_convert -> comp_output_capsfilter -> sink\n");
    //     return -1;
    // }

    // Связывание элементов
    if (!gst_element_link_many(compositor, post_conv, comp_output_capsfilter, sink, NULL))
    {
        g_printerr("Failed to link compositor chain\n");
        return -1;
    }

    // // Сначала свяжем композитор с конвертером
    // if (!gst_element_link(compositor, final_convert))
    // {
    //     g_printerr("Failed to link compositor -> final_convert\n");
    //     return -1;
    // }

    // // Затем конвертер с capsfilter
    // if (!gst_element_link(final_convert, comp_output_capsfilter))
    // {
    //     g_printerr("Failed to link final_convert -> comp_output_capsfilter\n");
    //     return -1;
    // }

    // // И наконец capsfilter с sink
    // if (!gst_element_link(comp_output_capsfilter, sink))
    // {
    //     g_printerr("Failed to link comp_output_capsfilter -> sink\n");
    //     return -1;
    // }

    gst_element_set_state(pipeline, GST_STATE_READY);
    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_printerr("Failed to set pipeline to playing state\n");
        return -1;
    }

    bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_add_watch(bus, bus_call, loop);
    gst_object_unref(bus);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    g_main_loop_run(loop);

    // Правильное завершение
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    g_main_loop_unref(loop);

    if (compositor_src_pad)
        gst_object_unref(compositor_src_pad);
    if (capsfilter_sink_pad)
        gst_object_unref(capsfilter_sink_pad);

      return 0;
}