#include <esp32camera.h>
#include "fd_forward.h"

#include <SPI.h>
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();
#include <TFT_eFEX.h>
TFT_eFEX fex = TFT_eFEX(&tft);

#define ESP32CAMERA_FRAMESIZE    FRAMESIZE_QQVGA //FRAMESIZE_QCIF
#define ESP32CAMERA_JPEG_QUALITY 10
#define ESP32CAMERA_FPS          10

esp32camera mainCamera;

static mtmn_config_t mtmn_config = {0};

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
  tft.setRotation(2);  // 0 & 2 Portrait. 1 & 3 landscape
  tft.fillScreen(TFT_BLACK);

  
  mtmn_config.type = FAST;
  mtmn_config.min_face = 80;
  mtmn_config.pyramid = 0.707;
  mtmn_config.pyramid_times = 4;
  mtmn_config.p_threshold.score = 0.6;
  mtmn_config.p_threshold.nms = 0.7;
  mtmn_config.p_threshold.candidate_number = 20;
  mtmn_config.r_threshold.score = 0.7;
  mtmn_config.r_threshold.nms = 0.7;
  mtmn_config.r_threshold.candidate_number = 10;
  mtmn_config.o_threshold.score = 0.7;
  mtmn_config.o_threshold.nms = 0.7;
  mtmn_config.o_threshold.candidate_number = 1;

  mainCamera.addTask(showImage, true);
  mainCamera.addTask(showFace, true);
}

/**
 * Task to display image on screen
 */
void showImage(camera_fb_t *_fb)
{
  fex.drawJpg((const uint8_t*)_fb->buf, _fb->len, 80, 80);
}

void showFace(camera_fb_t *_fb)
{
  int x, y, w, h, i;

  dl_matrix3du_t *image_matrix = NULL;
  image_matrix = dl_matrix3du_alloc(1, _fb->width, _fb->height, 3);
  if (!image_matrix) {
    Serial.println("dl_matrix3du_alloc failed");
    return;
  }
  if (!fmt2rgb888(_fb->buf, _fb->len, _fb->format, image_matrix->item)) {
    Serial.println("fmt2rgb888 failed");
    dl_matrix3du_free(image_matrix);
    return;
  }
  box_array_t *net_boxes = NULL;
  net_boxes = face_detect(image_matrix, &mtmn_config);

  if (net_boxes){
    Serial.println("Face detected");
    i = 0;
    
    x = (int)net_boxes->box[i].box_p[0];
    y = (int)net_boxes->box[i].box_p[1];
    w = (int)net_boxes->box[i].box_p[2] - x + 1;
    h = (int)net_boxes->box[i].box_p[3] - y + 1;

    Serial.printf("At %u,%u size %u,%u\n", x, y, w, h);
    fex.drawJpg((const uint8_t*)_fb->buf, _fb->len, 0, 80, w, h, x, y);
    Serial.println("Done");

    // We should clear, but my esp32 failed here =(
    //free(net_boxes->score);
    //free(net_boxes->box);
    //free(net_boxes->landmark);
    //free(net_boxes);
    //delay(100);
  }
  dl_matrix3du_free(image_matrix);
}

void loop()
{
  mainCamera.processFrame();
}
