# ESP32 camera wrapper with tasks

## Face detection issue
face detection will not work in new version of Arduino, see [here](https://github.com/easytarget/esp32-cam-webserver/issues/191)

## Usage

```cpp
esp32camera camera;

void sendFrame(camera_fb_t *fb) {
  // e.g. ws.binary(clientId, fb->buf, fb->len);
}

void setup() {
  // optional last two arguments: LEDC channel and timer for the camera clock,
  // pick ones your servos/PWM don't use (default LEDC_CHANNEL_0, LEDC_TIMER_0)
  if (camera.setup(FRAMESIZE_QVGA, 25, 12, LEDC_CHANNEL_7, LEDC_TIMER_3)) {
    camera.addTask(sendFrame);
  }
}

void loop() {
  camera.processFrame();   // grabs a frame every 1/fps and runs the tasks
}
```
