/**
 * ESP32 camera lib
 */
#include "esp32camera.h"

void esp32camera::setup(framesize_t frameSize, int fps, int quality) {
	_cameraLoop = 1000/(float)fps;  // 1000ms = 1 seconds, process frame every 1/FPS

	camera_config_t config;

	config.ledc_channel = LEDC_CHANNEL_0;
	config.ledc_timer   = LEDC_TIMER_0;
	config.pin_d0       = Y2_GPIO_NUM;
	config.pin_d1       = Y3_GPIO_NUM;
	config.pin_d2       = Y4_GPIO_NUM;
	config.pin_d3       = Y5_GPIO_NUM;
	config.pin_d4       = Y6_GPIO_NUM;
	config.pin_d5       = Y7_GPIO_NUM;
	config.pin_d6       = Y8_GPIO_NUM;
	config.pin_d7       = Y9_GPIO_NUM;
	config.pin_xclk     = XCLK_GPIO_NUM;
	config.pin_pclk     = PCLK_GPIO_NUM;
	config.pin_vsync    = VSYNC_GPIO_NUM;
	config.pin_href     = HREF_GPIO_NUM;
	config.pin_sscb_sda = SIOD_GPIO_NUM;
	config.pin_sscb_scl = SIOC_GPIO_NUM;
	config.pin_pwdn     = PWDN_GPIO_NUM;
	config.pin_reset    = RESET_GPIO_NUM;
	config.xclk_freq_hz = 20000000;
	config.pixel_format = PIXFORMAT_JPEG;

	config.frame_size   = frameSize; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA @see https://github.com/espressif/esp32-camera/blob/master/driver/include/sensor.h
	config.jpeg_quality = quality;

	if(psramFound()){
		config.fb_count = 2;
	} else {
		config.fb_count = 1;
	}

	// Init Camera
	esp_err_t err = esp_camera_init(&config);
	if (err != ESP_OK) {
		Serial.printf("ESP32CAM: Camera init failed with error 0x%x", err);
		return;
	}

	sensor_t * s = esp_camera_sensor_get();

	s->set_brightness(s, 0);     // -2 to 2
	s->set_contrast(s, 0);       // -2 to 2
	s->set_saturation(s, 0);     // -2 to 2
	s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
	s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
	s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
	s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
	s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
	s->set_aec2(s, 1);           // 0 = disable , 1 = enable
	s->set_ae_level(s, 0);       // -2 to 2
	s->set_aec_value(s, 300);    // 0 to 1200
	s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
	s->set_agc_gain(s, 0);       // 0 to 30
	s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
	s->set_bpc(s, 0);            // 0 = disable , 1 = enable
	s->set_wpc(s, 1);            // 0 = disable , 1 = enable
	s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
	s->set_lenc(s, 1);           // 0 = disable , 1 = enable
	s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
	s->set_vflip(s, 0);          // 0 = disable , 1 = enable
	s->set_dcw(s, 1);            // 0 = disable , 1 = enable
	s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
	
	s->set_framesize(s, frameSize);

	#ifdef ASYNCWEBSOCKET_H_
		addTask(sendFrame);
	#endif
}

#ifdef ASYNCWEBSOCKET_H_
	void esp32camera::setClientReady(AsyncWebSocketClient * &wsClient) {
		_clientReady = true;
		_wsClient = wsClient;
	}

	void esp32camera::setClientDisconnected() {
		_clientReady = false;
	}

	void esp32camera::sendFrame(camera_fb_t _fb)
	{
		if (_clientReady && _framebuffer) {
			_wsclient->binary(_framebuffer->buf, _framebuffer->len); // TODO should be something prior that `buf` to mark that this package is a frame
			
		}
	}
#endif

void esp32camera::processFrame()
{
	_cameraCurrentTime = millis();

	if (_cameraCurrentTime - _cameraPreviousTime >= _cameraLoop) {
		_fb = esp_camera_fb_get();

		for(uint8_t idx = 0; idx < ESP32CAMERA_MAX_TASK; idx++) {
			if (_tasks[idx].isEnabled && !_tasks[idx].isRemoved) {
				_tasks[idx].task(_fb);
			}
		}

		esp_camera_fb_return(_fb);
		_fb = NULL;

		_cameraPreviousTime = _cameraCurrentTime;
	}
}

uint8_t esp32camera::addTask(cameraProcessFunction _task, bool isEnabled)
{
	for(uint8_t idx = 0; idx < ESP32CAMERA_MAX_TASK; idx++) {
		if (_tasks[idx].isRemoved) {
			_tasks[idx].isEnabled = isEnabled;
			_tasks[idx].isRemoved = false;
			_tasks[idx].task      = _task;
		return idx;

		}
	}

	return ESP32CAMERA_ADD_TASK_FAILED;
}

bool esp32camera::removeTask(uint8_t idx)
{
	_tasks[idx].isRemoved = false;
}

bool esp32camera::setTaskState(uint8_t idx, bool isEnabled)
{
	if (_tasks[idx].isRemoved) {
		return false;
	}
	
	_tasks[idx].isEnabled = isEnabled;
	
	return true;
}

bool esp32camera::enableTask(uint8_t idx)
{
	return setTaskState(idx, true);
}

bool esp32camera::disableTask(uint8_t idx)
{
	return setTaskState(idx, false);
}
