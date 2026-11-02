#ifndef esp32camera_h
#define esp32camera_h

#include <Arduino.h>

/**
 * @TODO make it better
 */
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM


#define ESP32CAMERA_MAX_TASK         10
#define ESP32CAMERA_ADD_TASK_FAILED 255

#include "esp_camera.h"
#include "esp32camera_pins.h"

typedef void (*cameraProcessFunction)(camera_fb_t *_framebuffer);

typedef struct cameraProcessTasks_t {
	bool isEnabled = false;
	bool isRemoved = true;
	cameraProcessFunction task;
} cameraProcessTasks;

class esp32camera
{
	public:
		/**
		 * @param ledcChannel, ledcTimer LEDC channel/timer for the camera clock
		 *        (XCLK). Pick ones nothing else uses: on Arduino-ESP32 3.x
		 *        ledcAttach() allocates channels itself, starting from 0.
		 * @return false if the camera did not start
		 */
		bool setup(framesize_t frameSize, int fps, int quality,
		           ledc_channel_t ledcChannel = LEDC_CHANNEL_0, ledc_timer_t ledcTimer = LEDC_TIMER_0);
		// Grab a frame every 1/fps and hand it to the enabled tasks. Send it to a
		// websocket, file, display... from a task added with addTask().
		void processFrame();
		uint8_t addTask(cameraProcessFunction _task, bool isEnabled = true);
		bool removeTask(uint8_t idx);
		bool setTaskState(uint8_t idx, bool isEnabled);
		bool enableTask(uint8_t idx);
		bool disableTask(uint8_t idx);
	private:
		camera_fb_t          *_fb = NULL;
		unsigned long        _cameraCurrentTime;
		unsigned long        _cameraPreviousTime = 0;
		unsigned long        _cameraLoop;
		cameraProcessTasks   _tasks[ESP32CAMERA_MAX_TASK];
		bool                 _ready = false;
};
#endif
