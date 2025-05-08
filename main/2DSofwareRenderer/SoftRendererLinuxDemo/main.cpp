#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <sys/mman.h>
#include <chrono>
#include <thread>
#include <linux/input.h>
#include <SoftRendererLib/src/include/SoftRenderer.h>
#include <SoftRendererLib/src/data/PixelFormat/PixelFormatInfo.h>

#define STB_IMAGE_IMPLEMENTATION

#include "lib/stb_image.h"
#include <fstream>

using namespace Tergos2D;

#define CHECK_ERR(cond, msg) \
    if (cond)                \
    {                        \
        perror(msg);         \
        exit(EXIT_FAILURE);  \
    }

void handle_drm_events(int drm_fd)
{
    drmEventContext evctx = {
        .version = DRM_EVENT_CONTEXT_VERSION,
        .vblank_handler = nullptr,
        .page_flip_handler = nullptr,
    };

    struct pollfd fds = {
        .fd = drm_fd,
        .events = POLLIN,
    };

    while (poll(&fds, 1, -1) <= 0)
        ; // Wait for events

    if (fds.revents & POLLIN)
    {
        drmHandleEvent(drm_fd, &evctx);
    }
}

uint8_t *create_framebuffer(int drm_fd, uint32_t width, uint32_t height, uint32_t bpp, uint32_t &fb_id, uint32_t &handle, uint32_t &pitch, uint64_t &size)
{
    struct drm_mode_create_dumb create_dumb = {0};
    create_dumb.width = width;
    create_dumb.height = height;
    create_dumb.bpp = bpp;
    CHECK_ERR(drmIoctl(drm_fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_dumb) < 0, "Failed to create dumb buffer");

    handle = create_dumb.handle;
    pitch = create_dumb.pitch;
    size = create_dumb.size;

    struct drm_mode_map_dumb map_dumb = {0};
    map_dumb.handle = handle;
    CHECK_ERR(drmIoctl(drm_fd, DRM_IOCTL_MODE_MAP_DUMB, &map_dumb) < 0, "Failed to map dumb buffer");

    uint8_t *pixels = (uint8_t *)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, drm_fd, map_dumb.offset);
    CHECK_ERR(pixels == MAP_FAILED, "Failed to mmap dumb buffer");

    CHECK_ERR(drmModeAddFB(drm_fd, width, height, 24, 24, pitch, handle, &fb_id) < 0, "Failed to create framebuffer");

    return pixels;
}

double getCurrentTime()
{
    using Clock = std::chrono::steady_clock;
    static auto startTime = Clock::now();
    auto currentTime = Clock::now();
    auto duration = std::chrono::duration<double>(currentTime - startTime);
    return duration.count();
}

void handle_touch_events(int touch_fd)
{
    struct input_event ev;
    while (read(touch_fd, &ev, sizeof(ev)) > 0)
    {
        if (ev.type == EV_ABS)
        {
            if (ev.code == ABS_MT_POSITION_X)
            {
                printf("Touch X: %d\n", ev.value);
            }
            else if (ev.code == ABS_MT_POSITION_Y)
            {
                printf("Touch Y: %d\n", ev.value);
            }
        }
    }
}


void TestTexturePerformance(RenderContext2D& context) {
    // Texturgrößen von 30 bis 480 in ~50 Pixel Schritten
    std::vector<size_t> sizes = {30, 80, 130, 180, 230, 280, 330, 380, 430, 480};
    
    // Erstelle Testdaten für die größte Textur (480x480 RGBA8888)
    std::vector<uint8_t> maxTextureData(480 * 480 * 3, 255);
    
    std::cout << "Texture Size (WxH) | Time per 100 Draws (ms)" << std::endl;
    std::cout << "------------------------------------------------" << std::endl;
    
    for(size_t size : sizes) {
        // Erstelle Textur mit aktueller Größe
        std::vector<uint8_t> textureData(size * size * 4);
        // Fülle mit Testdaten
        for(size_t i = 0; i < textureData.size(); i += 4) {
            textureData[i] = 255;     // R
            textureData[i+1] = 0;     // G
            textureData[i+2] = 0;     // B
            textureData[i+3] = 255;   // A
        }
        
        Texture testTexture(size, size, textureData.data(), PixelFormat::ARGB8888, 0);
        
        // Zeitmessung Start
        auto start = std::chrono::high_resolution_clock::now();
        
        // 500 DrawTexture Aufrufe (100 * 5 wie im Original)
        for (size_t xPos = 0; xPos < 100; xPos++) {
            for (size_t yPos = 0; yPos < 1; yPos++) {
                context.basicTextureRenderer.DrawTexture(testTexture, 0, 0);
            }
        }
        
        // Zeitmessung Ende
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << size << "x" << size << "\t\t| " 
                 << duration.count() << " ms" << std::endl;
    }
}

int main()
{
    int drm_fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
    CHECK_ERR(drm_fd < 0, "Failed to open /dev/dri/card0");

    drmModeRes *resources = drmModeGetResources(drm_fd);
    CHECK_ERR(!resources, "Failed to get DRM resources");

    drmModeConnector *connector = nullptr;
    uint32_t connector_id = 0;
    for (int i = 0; i < resources->count_connectors; i++)
    {
        connector = drmModeGetConnector(drm_fd, resources->connectors[i]);
        if (connector && connector->connection == DRM_MODE_CONNECTED)
        {
            connector_id = connector->connector_id;
            break;
        }
        drmModeFreeConnector(connector);
    }
    CHECK_ERR(connector_id == 0, "No connected connector found");

    drmModeModeInfo mode = connector->modes[0];
    uint32_t width = mode.hdisplay;
    uint32_t height = mode.vdisplay;

    drmModeEncoder *encoder = drmModeGetEncoder(drm_fd, connector->encoder_id);
    CHECK_ERR(!encoder, "Failed to get encoder");

    drmModeCrtc *crtc = drmModeGetCrtc(drm_fd, encoder->crtc_id);

    uint32_t fb_id[2], handle[2], pitch[2];
    uint64_t size[2];
    uint8_t *framebuffer[2];

    for (int i = 0; i < 2; ++i)
    {
        framebuffer[i] = create_framebuffer(drm_fd, width, height, 24, fb_id[i], handle[i], pitch[i], size[i]);
    }

    RenderContext2D context;

    bool running = true;
    int current = 0;

    CHECK_ERR(drmModeSetCrtc(drm_fd, crtc->crtc_id, fb_id[current], 0, 0, &connector_id, 1, &mode) < 0, "Failed to set CRTC");

    Texture text;
    Texture text2;
    Texture text3;
    int imgwidth, imgheight, nrChannels;
    uint8_t *data = nullptr;
    uint8_t *data2 = nullptr;
    uint8_t *data3 = nullptr;
    Texture text4;
    uint8_t *data4 = nullptr;
    Texture text5;
    uint8_t *data5 = nullptr;
    int imgwidth4 = 234;
    int imgheight4 = 243;

    data = stbi_load("data/img1.jpg", &imgwidth, &imgheight, &nrChannels, 3);
    text = Texture(imgwidth, imgheight, data, PixelFormat::RGB24, 0);
    data2 = stbi_load("data/Candera.png", &imgwidth, &imgheight, &nrChannels, 4);
    text2 = Texture(imgwidth, imgheight, data2, PixelFormat::RGBA8888, 0);
    data3 = stbi_load("data/logo-de.png", &imgwidth, &imgheight, &nrChannels, 4);
    text3 = Texture(imgwidth, imgheight, data3, PixelFormat::RGBA8888, 0);
    data5 = stbi_load("data/images.png", &imgwidth, &imgheight, &nrChannels, 3);
    text5 = Texture(imgwidth, imgheight, data5, PixelFormat::RGB24, 0);

    std::ifstream file("data/testrgb565.bin", std::ios::binary | std::ios::ate);
    if (file)
    {
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        data4 = new uint8_t[size];
        if (file.read(reinterpret_cast<char *>(data4), size))
        {
            text4 = Texture(imgwidth4, imgheight4, data4, PixelFormat::RGB565, 0);
        }
        else
        {
            std::cerr << "Failed to read testrgb565.bin" << std::endl;
        }
        file.close();
    }
    else
    {
        std::cerr << "Failed to open testrgb565.bin" << std::endl;
    }
    context.EnableClipping(false);

    double previousTime = 0.0;
    int frameCount = 0;
    float x = 0.0f;

    int touch_fd = open("/dev/input/event0", O_RDONLY);
    CHECK_ERR(touch_fd < 0, "Failed to open /dev/input/event0");

    struct pollfd fds[2];
    fds[0].fd = drm_fd;
    fds[0].events = POLLIN;
    fds[1].fd = touch_fd;
    fds[1].events = POLLIN;

    while (running)
    {
        int next = 1 - current;

        Texture texture = Texture(width, height, framebuffer[next], PixelFormat::BGR24, pitch[next]);
        context.SetTargetTexture(&texture);

        context.ClearTarget(Color(150, 150, 150));
        context.SetClipping(80, 30, 370, 290);
        context.EnableClipping(false);
        Coloring coloring;
        coloring.color = Color(155,0,255,0);
        coloring.colorEnabled = false;
        context.SetColoringSettings(coloring);
    
        TestTexturePerformance(context);


        CHECK_ERR(drmModePageFlip(drm_fd, crtc->crtc_id, fb_id[next], DRM_MODE_PAGE_FLIP_EVENT, nullptr) < 0, "Failed to page flip");

        poll(fds, 2, -1);

        if (fds[0].revents & POLLIN)
        {
            handle_drm_events(drm_fd);
        }

        if (fds[1].revents & POLLIN)
        {
            handle_touch_events(touch_fd);
        }

        x += 0.5f;
        current = next;

        double currentTime = getCurrentTime();
        frameCount++;

        if (currentTime - previousTime >= 1.0)
        {
            double fps = frameCount / (currentTime - previousTime);
            double timePerFrame = 1000.0 / fps;
            std::cout << "FPS: " << fps << " | Time per frame: " << timePerFrame << " ms" << std::endl;

            previousTime = currentTime;
            frameCount = 0;
        }
    }

    close(touch_fd);

    for (int i = 0; i < 2; ++i)
    {
        munmap(framebuffer[i], size[i]);
        drmModeRmFB(drm_fd, fb_id[i]);
        struct drm_mode_destroy_dumb destroy_dumb = {0};
        destroy_dumb.handle = handle[i];
        drmIoctl(drm_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy_dumb);
    }

    drmModeFreeCrtc(crtc);
    drmModeFreeEncoder(encoder);
    drmModeFreeConnector(connector);
    drmModeFreeResources(resources);
    close(drm_fd);

    return 0;
}