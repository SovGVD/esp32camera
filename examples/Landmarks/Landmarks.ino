#include <esp32camera.h>
#include "fd_forward.h"

#define ESP32CAMERA_FRAMESIZE    FRAMESIZE_QQVGA //FRAMESIZE_QCIF
#define ESP32CAMERA_JPEG_QUALITY 10
#define ESP32CAMERA_FPS          10

esp32camera mainCamera;

static mtmn_config_t mtmn_config = {0};

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  mainCamera.setup(
    ESP32CAMERA_FRAMESIZE,
    ESP32CAMERA_FPS,
    ESP32CAMERA_JPEG_QUALITY
  );

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

  mainCamera.addTask(printLandmarks, true);
}

void printLandmarks(camera_fb_t *_fb)
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
    i = 0;

    x = (int)net_boxes->box[i].box_p[0];
    y = (int)net_boxes->box[i].box_p[1];
    w = (int)net_boxes->box[i].box_p[2] - x + 1;
    h = (int)net_boxes->box[i].box_p[3] - y + 1;

    Serial.printf("At %u,%u size %u,%u\n", x, y, w, h);
    Serial.printf("Landmarks: %u,%u,%u,%u,%u,%u,%u,%u,%u,%u ",
        (int)net_boxes->landmark->landmark_p[0],
        (int)net_boxes->landmark->landmark_p[1],
        (int)net_boxes->landmark->landmark_p[2],
        (int)net_boxes->landmark->landmark_p[3],
        (int)net_boxes->landmark->landmark_p[4],
        (int)net_boxes->landmark->landmark_p[5],
        (int)net_boxes->landmark->landmark_p[6],
        (int)net_boxes->landmark->landmark_p[7],
        (int)net_boxes->landmark->landmark_p[8],
        (int)net_boxes->landmark->landmark_p[9]
    );
  }
  dl_matrix3du_free(image_matrix);
}

void loop()
{
  mainCamera.processFrame();
}
