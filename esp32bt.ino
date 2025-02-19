/* Edge Impulse + Bluetooth Serial for ESP32-CAM */

#include <Anunay6827-project-1_inferencing.h>
#include "edge-impulse-sdk/dsp/image/image.hpp"
#include "esp_camera.h"
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;  // Initialize Bluetooth Serial

#define CAMERA_MODEL_AI_THINKER  // Camera Model

// Camera Pin Configuration for AI Thinker Model
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Camera Configuration
static camera_config_t camera_config = {
    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_sscb_sda = SIOD_GPIO_NUM,
    .pin_sscb_scl = SIOC_GPIO_NUM,
    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,
    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE_QVGA,
    .jpeg_quality = 12,
    .fb_count = 1,
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

void setup() {
    Serial.begin(9600);
    SerialBT.begin("ESP32-CAM");  // Start Bluetooth Serial
    Serial.println("ESP32-CAM Bluetooth Initialized");
    SerialBT.println("ESP32-CAM Bluetooth Initialized");

    if (esp_camera_init(&camera_config) != ESP_OK) {
        Serial.println("Camera initialization failed!");
        SerialBT.println("Camera initialization failed!");
        return;
    }
    Serial.println("Camera initialized successfully");
    SerialBT.println("Camera initialized successfully");

    delay(2000);
}

void loop() {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        SerialBT.println("Camera capture failed");
        return;
    }

    // Object Detection Processing
    ei::signal_t signal;
    ei_impulse_result_t result = {0};
    signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
    signal.get_data = &ei_camera_get_data;

    EI_IMPULSE_ERROR err = run_classifier(&signal, &result, false);
    if (err != EI_IMPULSE_OK) {
        Serial.printf("Error running classifier: %d\n", err);
        SerialBT.printf("Error running classifier: %d\n", err);
        return;
    }

    Serial.println("Object Detection Results:");
    SerialBT.println("Object Detection Results:");

    // Print Bounding Boxes
    for (uint32_t i = 0; i < result.bounding_boxes_count; i++) {
        ei_impulse_result_bounding_box_t bb = result.bounding_boxes[i];
        if (bb.value == 0) continue;

        Serial.printf("  %s (%.2f) [x:%d, y:%d, w:%d, h:%d]\n", 
                      bb.label, bb.value, bb.x, bb.y, bb.width, bb.height);
        SerialBT.printf("  %s (%.2f) [x:%d, y:%d, w:%d, h:%d]\n", 
                        bb.label, bb.value, bb.x, bb.y, bb.width, bb.height);
    }

    // Print Classification Results
    Serial.println("Classification Results:");
    SerialBT.println("Classification Results:");
    for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        Serial.printf("  %s: %.5f\n", ei_classifier_inferencing_categories[i], result.classification[i].value);
        SerialBT.printf("  %s: %.5f\n", ei_classifier_inferencing_categories[i], result.classification[i].value);
    }

    esp_camera_fb_return(fb);
    delay(5000);
}

// Function to Get Image Data
static int ei_camera_get_data(size_t offset, size_t length, float *out_ptr) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Failed to capture image!");
        SerialBT.println("Failed to capture image!");
        return -1;
    }

    size_t pixel_ix = offset * 3;
    size_t pixels_left = length;
    size_t out_ptr_ix = 0;

    while (pixels_left != 0 && pixel_ix < fb->len) {
        out_ptr[out_ptr_ix] = (fb->buf[pixel_ix + 2] << 16) + (fb->buf[pixel_ix + 1] << 8) + fb->buf[pixel_ix];
        out_ptr_ix++;
        pixel_ix += 3;
        pixels_left--;
    }

    esp_camera_fb_return(fb);
    return 0;
}

