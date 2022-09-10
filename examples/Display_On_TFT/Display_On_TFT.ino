#include <esp32camera.h>

#include <SPI.h>
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();
#include <TFT_eFEX.h>
TFT_eFEX fex = TFT_eFEX(&tft);

#define ESP32CAMERA_FRAMESIZE    FRAMESIZE_HQVGA
#define ESP32CAMERA_JPEG_QUALITY 10
#define ESP32CAMERA_FPS          5

esp32camera mainCamera;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  mainCamera.setup(
    ESP32CAMERA_FRAMESIZE,
    ESP32CAMERA_FPS,
    ESP32CAMERA_JPEG_QUALITY
  );

  /**
   * Init screen
   */
  tft.begin();
  tft.setRotation(0);  // 0 & 2 Portrait. 1 & 3 landscape
  tft.fillScreen(TFT_BLACK);

  mainCamera.addTask(showImage, true);
}

/**
 * Task to display image on screen
 */
void showImage(camera_fb_t *_fb)
{
  fex.drawJpg((const uint8_t*)_fb->buf, _fb->len, 0, 0);
}

void loop()
{
  mainCamera.processFrame();
}
