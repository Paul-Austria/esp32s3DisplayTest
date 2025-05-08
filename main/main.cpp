extern "C"{
    #include <stdio.h>
    #include "sdkconfig.h"
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "freertos/semphr.h"
    #include "esp_heap_caps.h"
    #include "esp_log.h"
    #include "TCA9554PWR.h"
    #include "PCF85063.h"
    #include "QMI8658.h"
    #include "ST7701S.h"
    #include "GT911.h"
    #include "Wireless.h"
    #include "BAT_Driver.h"
    #include "esp_timer.h"
    #include <cmath>
    }

static const char *TAG = "DisplayTest";
#include "SoftRenderer.h"
using namespace Tergos2D;

RenderContext2D context;

#define SCREEN_WIDTH   480
#define SCREEN_HEIGHT  480
#define SCREEN_BITS    16  // RGB565
#define FRAME_SIZE    (SCREEN_WIDTH * SCREEN_HEIGHT * (SCREEN_BITS/8))

void *front_buffer = NULL;
void *back_buffer = NULL;




uint16_t hue = 0;
int square_x = 0, square_y = 0;
int move_direction = 1;

#include <cmath>
#include <cstdlib>

struct Square {
    float x, y;
    float dx, dy;
    float hue;
    static const int SIZE = 50;
};

const int amount = 200;
static Square squares[amount];
static bool initialized = false;



//random test to fill the screen with data
void fill_screen() {
    static Texture texture;
    texture = Texture(480,480,(uint8_t*)back_buffer,PixelFormat::RGB565);
    context.ClearTarget(Color(155,155,155));
    context.SetTargetTexture(&texture);

    if (!initialized) {
        for (int i = 0; i < amount; i++) {
            squares[i].x = rand() % (480 - Square::SIZE);
            squares[i].y = rand() % (480 - Square::SIZE);
            squares[i].dx = (rand() % 5 - 2) * 0.9f;
            squares[i].dy = (rand() % 5 - 2) * 0.9f;
            squares[i].hue = rand() % 360;
        }
        initialized = true;
    }

    for (int i = 0; i < amount; i++) {
        squares[i].x += squares[i].dx;
        squares[i].y += squares[i].dy;

        if (squares[i].x <= 0 || squares[i].x >= 480 - Square::SIZE) {
            squares[i].dx *= -1;
            squares[i].x = std::max(0.0f, std::min(squares[i].x, 480.0f - Square::SIZE));
        }
        if (squares[i].y <= 0 || squares[i].y >= 480 - Square::SIZE) {
            squares[i].dy *= -1;
            squares[i].y = std::max(0.0f, std::min(squares[i].y, 480.0f - Square::SIZE));
        }

        squares[i].hue += 0.5f;
        if (squares[i].hue >= 360.0f) squares[i].hue = 0;

        float s = 1.0f;
        float v = 1.0f;
        float c = v * s;
        float x = c * (1 - std::fabs(std::fmod(squares[i].hue / 60.0f, 2.0f) - 1));
        float m = v - c;

        float r = 0, g = 0, b = 0;

        int hue_section = static_cast<int>(squares[i].hue / 60.0f);
        switch (hue_section) {
            case 0: r = c; g = x; b = 0; break;
            case 1: r = x; g = c; b = 0; break;
            case 2: r = 0; g = c; b = x; break;
            case 3: r = 0; g = x; b = c; break;
            case 4: r = x; g = 0; b = c; break;
            default: r = c; g = 0; b = x; break;
        }

        uint8_t red = (uint8_t)((r + m) * 255);
        uint8_t green = (uint8_t)((g + m) * 255);
        uint8_t blue = (uint8_t)((b + m) * 255);

        // Draw the square
        context.primitivesRenderer.DrawRect(
            Color(red, green, blue),
            static_cast<int>(squares[i].x),
            static_cast<int>(squares[i].y),
            Square::SIZE,
            Square::SIZE
        );
    }
}

void update_display(void) {
    xSemaphoreGive(sem_gui_ready);
    xSemaphoreTake(sem_vsync_end, portMAX_DELAY);
    esp_err_t ret = esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, back_buffer);
   void *temp = front_buffer;
    front_buffer = back_buffer;
    back_buffer = temp;
}


static void display_test_task(void *pvParameters) {
    // Variables for FPS calculation
    uint32_t frame_count = 0;
    int64_t last_fps_time = 0;
    float fps = 0;
    float frame_time_ms = 0;

    int64_t start_time = esp_timer_get_time();
    last_fps_time = start_time;

    while (1) {
        int64_t frame_start = esp_timer_get_time();

        // Render frame on the back buffer
         // Swap front and back buffers

        fill_screen();
        update_display();

        int64_t frame_end = esp_timer_get_time();
        frame_time_ms = (frame_end - frame_start) / 1000.0f;
        frame_count++;

        if (frame_end - last_fps_time >= 1000000) {
            fps = frame_count * 1000000.0f / (frame_end - last_fps_time);

            ESP_LOGI(TAG, "FPS: %.1f, Frame Time: %.2fms", fps, frame_time_ms);

            frame_count = 0;
            last_fps_time = frame_end;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

static bool init_display(void) {
    esp_lcd_rgb_panel_get_frame_buffer(panel_handle, 2, &front_buffer,&back_buffer);
    if (front_buffer == NULL || back_buffer == NULL) {


        ESP_LOGE(TAG, "Failed to allocate framebuffers of size %d bytes", FRAME_SIZE);
        return false;
    }

    ESP_LOGI(TAG, "Successfully allocated framebuffers of size %d bytes each", FRAME_SIZE);

    // Clear both buffers
    memset(front_buffer, 0, FRAME_SIZE);
    memset(back_buffer, 0, FRAME_SIZE);

    // Set backlight to 75%
    Set_Backlight(75);

    // Create display test task
    BaseType_t task_created = xTaskCreatePinnedToCore(
        display_test_task,
        "disp_test",
        8096,
        NULL,
        configMAX_PRIORITIES - 1,
        NULL,
        0
    );

    if (task_created != pdPASS) {
        ESP_LOGE(TAG, "Failed to create display test task");
        heap_caps_free(front_buffer);
        heap_caps_free(back_buffer);
        front_buffer = NULL;
        back_buffer = NULL;
        return false;
    }

    return true;
}

static void driver_loop_task(void *parameter) {
    while(1) {
        QMI8658_Loop();
        RTC_Loop();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

static void init_drivers(void) {
    I2C_Init();
    PCF85063_Init();
    QMI8658_Init();
    EXIO_Init();

    xTaskCreatePinnedToCore(
        driver_loop_task,
        "driver_task",
        4096,
        NULL,
        3,
        NULL,
        0
    );
}

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "Starting display application");

    // Initialize all required systems
    Wireless_Init();
    init_drivers();
    LCD_Init();
    Touch_Init();

    // Initialize display and framebuffers
    if (!init_display()) {
        ESP_LOGE(TAG, "Display initialization failed");
        return;
    }

    ESP_LOGI(TAG, "Initialization complete - running display test");
}