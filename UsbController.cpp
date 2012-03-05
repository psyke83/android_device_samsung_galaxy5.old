/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>


#include <netinet/in.h>
#include <arpa/inet.h>

#define LOG_TAG "UsbController"
#include <cutils/log.h>
#include <cutils/properties.h>
#include <string.h>

#include "UsbController.h"


UsbController::UsbController() {
}

UsbController::~UsbController() {
}

int UsbController::startRNDIS() {
	LOGD("Usb RNDIS start");
	return enableRNDIS(true);
}

int UsbController::stopRNDIS() {
	LOGD("Usb RNDIS stop");
	return enableRNDIS(false);
}

int UsbController::enableRNDIS(bool enable) {
	char ums;
	int fdums = open("/sys/devices/platform/msm_hsusb/gadget/lun0/file", O_RDWR);
	read(fdums, &ums, 1);
	close(fdums);
	if (ums == '/')
		return 0;

	char value[20];
	int fd = open("/sys/devices/platform/android_usb/composition", O_RDWR);
	int count = snprintf(value, sizeof(value), "%s\n", (enable ? "6881" : "689e"));
	write(fd, value, count);
	close(fd);
	return 0;
}

bool UsbController::isRNDISStarted() {
    char value[5];
    int fd = open("/sys/module/g_android/parameters/product_id", O_RDONLY);
    read(fd, &value, 5);
    close(fd);
    return (!strncmp(value,"6881",4) ? true : false);
}
