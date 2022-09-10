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
		void setup(framesize_t frameSize, int fps, int quality);
		void processFrame();
		uint8_t addTask(cameraProcessFunction _task, bool isEnabled = true);
		bool removeTask(uint8_t idx);
		bool setTaskState(uint8_t idx, bool isEnabled);
		bool enableTask(uint8_t idx);
		bool disableTask(uint8_t idx);
		#ifdef ASYNCWEBSOCKET_H_
			void setClientReady(AsyncWebSocketClient * &wsClient);
			void setClientDisconnected();
			void sendFrame(camera_fb_t _framebuffer);
		#endif
	private:
		camera_fb_t          *_fb = NULL;
		unsigned long        _cameraCurrentTime;
		unsigned long        _cameraPreviousTime;
		unsigned long        _cameraLoop;
		uint8_t              _lastTask = 0;
		cameraProcessTasks   _tasks[ESP32CAMERA_MAX_TASK];
		#ifdef ASYNCWEBSOCKET_H_
			AsyncWebSocketClient *_wsclient;
			bool                 _clientReady = false;
		#endif
};
#endif
