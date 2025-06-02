#!/usr/bin/env bash
set -e
CWD=$(pwd -P)

ARDUINO_PATH="${CWD}/arduino-cli-ide"
ARDUINO_BIN_DIR="${ARDUINO_PATH}/bin"
ARDUINO_BIN="${ARDUINO_BIN_DIR}/arduino-cli"
ARDUINO_CONFIG="${ARDUINO_PATH}/arduino-ide-esp32.yaml"
ARDUINO_DATA="${ARDUINO_PATH}/.arduino"
ARDUINO_DOWNLOADS="${ARDUINO_PATH}/.arduino/staging"
ARDUINO_USER="${ARDUINO_PATH}/Arduino"
ARDUINO_LIB="${ARDUINO_USER}/libraries"
ARDUINO_BUILD="${ARDUINO_PATH}/.arduino/build"
ARDUINO_CLI="${ARDUINO_BIN} --no-color --config-file ${ARDUINO_CONFIG}"

EXAMPLES_DIR="${CWD}/examples"
EXAMPLE_NAME=""

COMMAND="$1"
PARAM1="$2"
PARAM2="$3"

installArduinoCli()
{
	# Create folders for Arduino CLI
	mkdir -p $ARDUINO_BIN_DIR
	curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=$ARDUINO_BIN_DIR sh -s 1.2.0
}

buildEnv() {
	if [ ! -f "$ARDUINO_BIN" ]; then
		installArduinoCli
	fi
	# Create folders for Arduino
	mkdir -p $ARDUINO_PATH
	mkdir -p $ARDUINO_DATA
	mkdir -p $ARDUINO_DOWNLOADS
	mkdir -p $ARDUINO_USER
	mkdir -p $ARDUINO_BUILD
	mkdir -p $ARDUINO_LIB

	echo "Build environment in: $CWD"
	echo "Arduino path: $ARDUINO_PATH"

	$ARDUINO_CLI config set board_manager.additional_urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
	echo "config: esp packages"
	$ARDUINO_CLI config set directories.data $ARDUINO_DATA
	echo "config: dir data"
	$ARDUINO_CLI config set directories.downloads $ARDUINO_DOWNLOADS
	echo "config: dir downloads"
	$ARDUINO_CLI config set directories.user $ARDUINO_USER
	echo "config: dir use"
	$ARDUINO_CLI config set library.enable_unsafe_install true
	echo "config: set unsafe"
	$ARDUINO_CLI config dump
	echo "Config steps done"

	# Install ESP32
	if [ ! -d "$ARDUINO_DATA/packages/esp32/hardware/esp32/1.0.6" ]; then
		$ARDUINO_CLI core update-index
		$ARDUINO_CLI core install esp32:esp32@1.0.6
		$ARDUINO_CLI core list
		$ARDUINO_CLI board listall
		echo "ESP32 core done"
	fi

	$ARDUINO_CLI lib install "TFT_eSPI"@2.5.31
	echo "TFT_eSPI Installed"

	$ARDUINO_CLI lib install "JPEGDecoder"@1.8.1
	echo "JPEGDecoder Installed"

	cd $ARDUINO_LIB
	if [ ! -d "$ARDUINO_LIB/TFT_eFEX" ]; then
		git clone https://github.com/Bodmer/TFT_eFEX.git
	fi
	cd $ARDUINO_LIB/TFT_eFEX
	git checkout 2b127258d8ee35122074a874aa1354c597e73a71
	cd $ARDUINO_LIB
	echo "TFT_eFEX Installed"

	cd $CWD
}

setExampleAndBoard() {
	case $PARAM1 in
		spi-display)
			EXAMPLE_NAME="Display_On_TFT"
			BOARD="esp32:esp32:esp32cam"
			;;
		spi-display-face-detection)
			EXAMPLE_NAME="Display_On_TFT_face"
			BOARD="esp32:esp32:esp32cam"
			;;
		*)
			compileHelp
			exit 0
			;;
	esac
}

compile() {
	setExampleAndBoard
	# Copy latest source into library folder
	mkdir -p $ARDUINO_LIB/esp32camera
	cp -r $CWD/src/* $ARDUINO_LIB/esp32camera
	# Compile
	# TODO: exit on error!
	echo "Compile: ${ARDUINO_CLI} compile --fqbn ${BOARD} ${EXAMPLES_DIR}/${EXAMPLE_NAME}..."
	$ARDUINO_CLI cache clean
	$ARDUINO_CLI compile --fqbn $BOARD $EXAMPLES_DIR/$EXAMPLE_NAME -v --build-path $ARDUINO_BUILD
	#rm $SRC_CONFIG_MODEL
}

upload() {
	[[ -z "$PARAM2" ]] && { echo "Port required" ; exit 1; }
	compile
	echo "Uploading ${EXAMPLE_NAME}. Port:${PARAM2}..."
	$ARDUINO_CLI upload -p ${PARAM2} --fqbn $BOARD $EXAMPLES_DIR/$EXAMPLE_NAME -v --input-dir $ARDUINO_BUILD
}

monitor() {
	[[ -z "$PARAM1" ]] && { echo "Port required" ; exit 1; }
	echo "Press Ctrl-C to exit"
	$ARDUINO_CLI monitor -p ${PARAM1} -c baudrate=115200
}

helpHeader() {
	echo "ESP32 camera build command line interface."
	echo ""
}

help() {
	helpHeader
	echo "Usage:"
	echo "  ./build.sh [command]"
	echo ""
	echo "Examples:"
	echo "  ./build.sh <command> [flags...]"
	echo ""
	echo "Available Commands:"
	echo "  build-env           Build environment for ESP32 and install required libraries"
	echo "  compile             Compile ESP32 binary"
	echo "  upload              Compile and upload"
	echo "  cli                 Arduino monitor, terminal"
}

compileHelp() {
	helpHeader
	echo "Usage:"
	echo "  ./build.sh compile EXAMPLE_NAME"
	echo ""
	echo "Examples:"
	echo "  ./build.sh compile Display_On_TFT"
	echo ""
	echo "Available Commands:"
	echo "  spi-display"
	echo "  spi-display-face-detection"
}

case $COMMAND in
	build-env)
		buildEnv
		;;
	compile)
		compile
		;;
	upload)
		upload
		;;
	cli)
		monitor
		;;
	*)
		help
		;;
esac
