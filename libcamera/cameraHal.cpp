/*
 * Copyright (C) 2012, Raviprasad V Mummidi.
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

#define LOG_TAG "CameraHAL"

#include <CameraHardwareInterface.h>
#include <hardware/hardware.h>
#include <hardware/camera.h>
#include <binder/IMemory.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <linux/msm_mdp.h>
#include <ui/Rect.h>
#include <ui/GraphicBufferMapper.h>
#include <dlfcn.h>

#define NO_ERROR 0
#define GRALLOC_USAGE_PMEM_PRIVATE_ADSP GRALLOC_USAGE_PRIVATE_0
#define MSM_COPY_HW 1
#define HWA 1
#ifdef HWA
#include "qcom/display/libgralloc/gralloc_priv.h"
#else
#include "libhardware/modules/gralloc/gralloc_priv.h"
#endif

//#define LOGV LOGI

struct blitreq {
   unsigned int count;
   struct mdp_blit_req req;
};

/* Prototypes and extern functions. */
android::sp<android::CameraHardwareInterface> (*LINK_openCameraHardware)(int id);
int (*LINK_getNumberofCameras)(void);
void (*LINK_getCameraInfo)(int cameraId, struct camera_info *info);
int qcamera_device_open(const hw_module_t* module, const char* name,
                        hw_device_t** device);
int CameraHAL_GetNum_Cameras(void);
int CameraHAL_GetCam_Info(int camera_id, struct camera_info *info);

/* Global variables. */
camera_notify_callback         origNotify_cb    = NULL;
camera_data_callback           origData_cb      = NULL;
camera_data_timestamp_callback origDataTS_cb    = NULL;
camera_request_memory          origCamReqMemory = NULL;

android::String8          g_str;
android::CameraParameters camSettings;
preview_stream_ops_t      *mWindow = NULL;
android::sp<android::CameraHardwareInterface> qCamera;

static hw_module_methods_t camera_module_methods = {
   open: qcamera_device_open
};

camera_module_t HAL_MODULE_INFO_SYM = {
   common: {
      tag: HARDWARE_MODULE_TAG,
      version_major: 1,
      version_minor: 0,
      id: CAMERA_HARDWARE_MODULE_ID,
      name: "Camera HAL for ICS",
      author: "Raviprasad V Mummidi",
      methods: &camera_module_methods,
      dso: NULL,
      reserved: {0},
   },
   get_number_of_cameras: CameraHAL_GetNum_Cameras,
   get_camera_info: CameraHAL_GetCam_Info,
};

/* HAL helper functions. */
void
CameraHAL_NotifyCb(int32_t msg_type, int32_t ext1,
                   int32_t ext2, void *user)
{
   LOGV("CameraHAL_NotifyCb: msg_type:%d ext1:%d ext2:%d user:%p\n",
        msg_type, ext1, ext2, user);
   if (origNotify_cb != NULL) {
      origNotify_cb(msg_type, ext1, ext2, user);
   }
}

bool
CameraHAL_CopyBuffers_Hw(int srcFd, int destFd,
                         size_t srcOffset, size_t destOffset,
                         int srcFormat, int destFormat,
                         int x, int y, int w, int h)
{
    struct blitreq blit;
    bool   success = true;
    int    fb_fd = open("/dev/graphics/fb0", O_RDWR);

#ifndef MSM_COPY_HW
    return false;
#endif

    if (fb_fd < 0) {
       LOGD("CameraHAL_CopyBuffers_Hw: Error opening /dev/graphics/fb0\n");
       return false;
    }

    LOGV("CameraHAL_CopyBuffers_Hw: srcFD:%d destFD:%d srcOffset:%#x"
         " destOffset:%#x x:%d y:%d w:%d h:%d\n", srcFd, destFd, srcOffset,
         destOffset, x, y, w, h);

    memset(&blit, 0, sizeof(blit));
    blit.count = 1;

    blit.req.flags       = 0;
    blit.req.alpha       = 0xff;
    blit.req.transp_mask = 0xffffffff;

    blit.req.src.width     = w;
    blit.req.src.height    = h;
    blit.req.src.offset    = srcOffset;
    blit.req.src.memory_id = srcFd;
    blit.req.src.format    = srcFormat;

    blit.req.dst.width     = w;
    blit.req.dst.height    = h;
    blit.req.dst.offset    = destOffset;
    blit.req.dst.memory_id = destFd;
    blit.req.dst.format    = destFormat;

    blit.req.src_rect.x = blit.req.dst_rect.x = x;
    blit.req.src_rect.y = blit.req.dst_rect.y = y;
    blit.req.src_rect.w = blit.req.dst_rect.w = w;
    blit.req.src_rect.h = blit.req.dst_rect.h = h;

    if (ioctl(fb_fd, MSMFB_BLIT, &blit)) {
       LOGV("CameraHAL_CopyBuffers_Hw: MSMFB_BLIT failed = %d %s\n",
            errno, strerror(errno));
       success = false;
    }
    close(fb_fd);
    return success;
}

void
CameraHal_Decode_Sw(unsigned int* rgb, char* yuv420sp, int width, int height)
{
   int frameSize = width * height;

   if (!qCamera->previewEnabled()) return;

   for (int j = 0, yp = 0; j < height; j++) {
      int uvp = frameSize + (j >> 1) * width, u = 0, v = 0;
      for (int i = 0; i < width; i++, yp++) {
         int y = (0xff & ((int) yuv420sp[yp])) - 16;
         if (y < 0) y = 0;
         if ((i & 1) == 0) {
            v = (0xff & yuv420sp[uvp++]) - 128;
            u = (0xff & yuv420sp[uvp++]) - 128;
         }

         int y1192 = 1192 * y;
         int r = (y1192 + 1634 * v);
         int g = (y1192 - 833 * v - 400 * u);
         int b = (y1192 + 2066 * u);

         if (r < 0) r = 0; else if (r > 262143) r = 262143;
         if (g < 0) g = 0; else if (g > 262143) g = 262143;
         if (b < 0) b = 0; else if (b > 262143) b = 262143;

         rgb[yp] = 0xff000000 | ((b << 6) & 0xff0000) |
                   ((g >> 2) & 0xff00) | ((r >> 10) & 0xff);
      }
   }
}



void
CameraHAL_CopyBuffers_Sw(char *dest, char *src, int size)
{
   int       i;
   int       numWords  = size / sizeof(unsigned);
   unsigned *srcWords  = (unsigned *)src;
   unsigned *destWords = (unsigned *)dest;

   for (i = 0; i < numWords; i++) {
      if ((i % 8) == 0 && (i + 8) < numWords) {
         __builtin_prefetch(srcWords  + 8, 0, 0);
         __builtin_prefetch(destWords + 8, 1, 0);
      }
      *destWords++ = *srcWords++;
   }
   if (__builtin_expect((size - (numWords * sizeof(unsigned))) > 0, 0)) {
      int numBytes = size - (numWords * sizeof(unsigned));
      char *destBytes = (char *)destWords;
      char *srcBytes  = (char *)srcWords;
      for (i = 0; i < numBytes; i++) {
         *destBytes++ = *srcBytes++;
      }
   }
}

void
CameraHAL_HandlePreviewData(const android::sp<android::IMemory>& dataPtr,
                            preview_stream_ops_t *mWindow,
                            camera_request_memory getMemory,
                            int32_t previewWidth, int32_t previewHeight)
{
   if (mWindow != NULL && getMemory != NULL) {
      ssize_t  offset;
      size_t   size;
      int32_t  previewFormat = MDP_Y_CBCR_H2V2;
#ifdef HWA
      int32_t  destFormat    = MDP_RGBX_8888;
#else
      int32_t  destFormat    = MDP_RGBA_8888;
#endif

      android::status_t retVal;
      android::sp<android::IMemoryHeap> mHeap = dataPtr->getMemory(&offset,
                                                                   &size);

      LOGV("CameraHAL_HandlePreviewData: previewWidth:%d previewHeight:%d "
           "offset:%#x size:%#x base:%p\n", previewWidth, previewHeight,
           (unsigned)offset, size, mHeap != NULL ? mHeap->base() : 0);

      mWindow->set_usage(mWindow,
#ifndef HWA
                         GRALLOC_USAGE_PMEM_PRIVATE_ADSP |
#endif
                         GRALLOC_USAGE_SW_READ_OFTEN);
      retVal = mWindow->set_buffers_geometry(mWindow,
                                             previewWidth, previewHeight,
#ifdef HWA
                                             HAL_PIXEL_FORMAT_RGBX_8888
#else
                                             HAL_PIXEL_FORMAT_RGBA_8888
#endif
                                             );
      if (retVal == NO_ERROR) {
         int32_t          stride;
         buffer_handle_t *bufHandle = NULL;

         LOGV("CameraHAL_HandlePreviewData: dequeueing buffer\n");
         retVal = mWindow->dequeue_buffer(mWindow, &bufHandle, &stride);
         if (retVal == NO_ERROR) {
            retVal = mWindow->lock_buffer(mWindow, bufHandle);
            if (retVal == NO_ERROR) {
               private_handle_t const *privHandle =
                  reinterpret_cast<private_handle_t const *>(*bufHandle);
               if (!CameraHAL_CopyBuffers_Hw(mHeap->getHeapID(), privHandle->fd,
                                             offset, privHandle->offset,
                                             previewFormat, destFormat,
                                             0, 0, previewWidth,
                                             previewHeight)) {
                  void *bits;
                  android::Rect bounds;
                  android::GraphicBufferMapper &mapper =
                     android::GraphicBufferMapper::get();

                  bounds.left   = 0;
                  bounds.top    = 0;
                  bounds.right  = previewWidth;
                  bounds.bottom = previewHeight;

                  mapper.lock(*bufHandle, GRALLOC_USAGE_SW_READ_OFTEN, bounds,
                              &bits);
                  LOGV("CameraHAL_HPD: w:%d h:%d bits:%p",
                       previewWidth, previewHeight, bits);
                  CameraHal_Decode_Sw((unsigned int *)bits, (char *)mHeap->base() + offset,
                                      previewWidth, previewHeight);

                  // unlock buffer before sending to display
                  mapper.unlock(*bufHandle);
               }

               mWindow->enqueue_buffer(mWindow, bufHandle);
               LOGV("CameraHAL_HandlePreviewData: enqueued buffer\n");
            } else {
               LOGE("CameraHAL_HandlePreviewData: ERROR locking the buffer\n");
               mWindow->cancel_buffer(mWindow, bufHandle);
            }
         } else {
            LOGE("CameraHAL_HandlePreviewData: ERROR dequeueing the buffer\n");
         }
      }
   }
}

camera_memory_t *
CameraHAL_GenClientData(const android::sp<android::IMemory> &dataPtr,
                        camera_request_memory reqClientMemory,
                        void *user)
{
   ssize_t          offset;
   size_t           size;
   camera_memory_t *clientData = NULL;
   android::sp<android::IMemoryHeap> mHeap = dataPtr->getMemory(&offset, &size);

   LOGV("CameraHAL_GenClientData: offset:%#x size:%#x base:%p\n",
        (unsigned)offset, size, mHeap != NULL ? mHeap->base() : 0);

   clientData = reqClientMemory(-1, size, 1, user);
   if (clientData != NULL) {
      CameraHAL_CopyBuffers_Sw((char *)clientData->data,
                               (char *)(mHeap->base()) + offset, size);
   } else {
      LOGV("CameraHAL_GenClientData: ERROR allocating memory from client\n");
   }
   return clientData;
}

void
CameraHAL_DataCb(int32_t msg_type, const android::sp<android::IMemory>& dataPtr,
                 void *user)
{
   LOGV("CameraHAL_DataCb: msg_type:%d user:%p\n", msg_type, user);
   if (msg_type == CAMERA_MSG_PREVIEW_FRAME) {
      int32_t previewWidth, previewHeight;
      android::CameraParameters hwParameters = qCamera->getParameters();
      hwParameters.getPreviewSize(&previewWidth, &previewHeight);
      CameraHAL_HandlePreviewData(dataPtr, mWindow, origCamReqMemory,
                                  previewWidth, previewHeight);
   } else if (origData_cb != NULL && origCamReqMemory != NULL) {
      camera_memory_t *clientData = CameraHAL_GenClientData(dataPtr,
                                       origCamReqMemory, user);
      if (clientData != NULL) {
         LOGV("CameraHAL_DataCb: Posting data to client\n");
         origData_cb(msg_type, clientData, 0, NULL, user);
         clientData->release(clientData);
      }
   }
}

void
CameraHAL_DataTSCb(nsecs_t timestamp, int32_t msg_type,
                   const android::sp<android::IMemory>& dataPtr, void *user)
{
   LOGV("CameraHAL_DataTSCb: timestamp:%lld msg_type:%d user:%p\n",
        timestamp /1000, msg_type, user);

   if (origDataTS_cb != NULL && origCamReqMemory != NULL) {
      camera_memory_t *clientData = CameraHAL_GenClientData(dataPtr,
                                       origCamReqMemory, user);
      if (clientData != NULL) {
         LOGV("CameraHAL_DataTSCb: Posting data to client timestamp:%lld\n",
              systemTime());
         origDataTS_cb(timestamp, msg_type, clientData, 0, user);
         qCamera->releaseRecordingFrame(dataPtr);
         clientData->release(clientData);
      } else {
         LOGD("CameraHAL_DataTSCb: ERROR allocating memory from client\n");
      }
   }
}

int
CameraHAL_GetNum_Cameras(void)
{
   int numCameras = 1;

   LOGE("CameraHAL_GetNum_Cameras:\n");
   void *libcameraHandle = ::dlopen("libcamera.so", RTLD_NOW);
   LOGD("CameraHAL_GetNum_Cameras: loading libcamera at %p", libcameraHandle);
   if (!libcameraHandle) {
       LOGE("FATAL ERROR: could not dlopen libcamera.so: %s", dlerror());
   } else {
      if (::dlsym(libcameraHandle, "HAL_getNumberOfCameras") != NULL) {
         *(void**)&LINK_getNumberofCameras =
                  ::dlsym(libcameraHandle, "HAL_getNumberOfCameras");
         numCameras = LINK_getNumberofCameras();
         LOGD("CameraHAL_GetNum_Cameras: numCameras:%d", numCameras);
      }
      dlclose(libcameraHandle);
   }
   return numCameras;
}

int
CameraHAL_GetCam_Info(int camera_id, struct camera_info *info)
{
   bool dynamic = false;
   LOGV("CameraHAL_GetCam_Info:\n");
   void *libcameraHandle = ::dlopen("libcamera.so", RTLD_NOW);
   LOGD("CameraHAL_GetNum_Cameras: loading libcamera at %p", libcameraHandle);
   if (!libcameraHandle) {
       LOGE("FATAL ERROR: could not dlopen libcamera.so: %s", dlerror());
       return EINVAL;
   } else {
      if (::dlsym(libcameraHandle, "HAL_getCameraInfo") != NULL) {
         *(void**)&LINK_getCameraInfo =
                  ::dlsym(libcameraHandle, "HAL_getCameraInfo");
         LINK_getCameraInfo(camera_id, info);
         dynamic = true;
      }
      dlclose(libcameraHandle);
   }
   if (!dynamic) {
      info->facing      = CAMERA_FACING_BACK;
      info->orientation = 90;
   }
   return NO_ERROR;
}

void
CameraHAL_FixupParams(android::CameraParameters &settings)
{
   const char *preview_sizes =
      "1280x720,800x480,768x432,720x480,640x480,576x432,480x320,384x288,352x288,320x240,240x160,176x144";
   const char *video_sizes =
      "1280x720,800x480,720x480,640x480,352x288,320x240,176x144";
   const char *preferred_size       = "640x480";
   const char *preview_frame_rates  = "30,27,24,15";
   const char *preferred_frame_rate = "15";
   const char *frame_rate_range     = "(15,30)";

   settings.set(android::CameraParameters::KEY_VIDEO_FRAME_FORMAT,
                android::CameraParameters::PIXEL_FORMAT_YUV420SP);

   if (!settings.get(android::CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES)) {
      settings.set(android::CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES,
                   preview_sizes);
   }

   if (!settings.get(android::CameraParameters::KEY_SUPPORTED_VIDEO_SIZES)) {
      settings.set(android::CameraParameters::KEY_SUPPORTED_VIDEO_SIZES,
                   video_sizes);
   }

   if (!settings.get(android::CameraParameters::KEY_VIDEO_SIZE)) {
      settings.set(android::CameraParameters::KEY_VIDEO_SIZE, preferred_size);
   }

   if (!settings.get(android::CameraParameters::KEY_PREFERRED_PREVIEW_SIZE_FOR_VIDEO)) {
      settings.set(android::CameraParameters::KEY_PREFERRED_PREVIEW_SIZE_FOR_VIDEO,
                   preferred_size);
   }

   if (!settings.get(android::CameraParameters::KEY_SUPPORTED_PREVIEW_FRAME_RATES)) {
      settings.set(android::CameraParameters::KEY_SUPPORTED_PREVIEW_FRAME_RATES,
                   preview_frame_rates);
   }

   if (!settings.get(android::CameraParameters::KEY_PREVIEW_FRAME_RATE)) {
      settings.set(android::CameraParameters::KEY_PREVIEW_FRAME_RATE,
                   preferred_frame_rate);
   }

   if (!settings.get(android::CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE)) {
      LOGD("Setting KEY_PREVIEW_FPS_RANGE: %s\n", frame_rate_range);
      settings.set(android::CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE,
                   frame_rate_range);
   }
}

/* Hardware Camera interface handlers. */
int
qcamera_set_preview_window(struct camera_device * device,
                           struct preview_stream_ops *window)
{
   LOGV("qcamera_set_preview_window : Window :%p\n", window);
   if (device == NULL) {
      LOGE("qcamera_set_preview_window : Invalid device.\n");
      return -EINVAL;
   } else {
      LOGV("qcamera_set_preview_window : window :%p\n", window);
      mWindow = window;
      return 0;
   }
}

void
qcamera_set_callbacks(struct camera_device * device,
                      camera_notify_callback notify_cb,
                      camera_data_callback data_cb,
                      camera_data_timestamp_callback data_cb_timestamp,
                      camera_request_memory get_memory, void *user)
{
   LOGV("qcamera_set_callbacks: notify_cb: %p, data_cb: %p "
        "data_cb_timestamp: %p, get_memory: %p, user :%p",
        notify_cb, data_cb, data_cb_timestamp, get_memory, user);

   origNotify_cb    = notify_cb;
   origData_cb      = data_cb;
   origDataTS_cb    = data_cb_timestamp;
   origCamReqMemory = get_memory;
   qCamera->setCallbacks(CameraHAL_NotifyCb, CameraHAL_DataCb,
                         CameraHAL_DataTSCb, user);
}

void
qcamera_enable_msg_type(struct camera_device * device, int32_t msg_type)
{
   LOGV("qcamera_enable_msg_type: msg_type:%#x\n", msg_type);
   if (msg_type == 0xfff) {
      msg_type = 0x1ff;
   } else {
      msg_type &= ~(CAMERA_MSG_PREVIEW_METADATA | CAMERA_MSG_RAW_IMAGE_NOTIFY);
   }
   qCamera->enableMsgType(msg_type);
}

void
qcamera_disable_msg_type(struct camera_device * device, int32_t msg_type)
{
   LOGV("qcamera_disable_msg_type: msg_type:%#x\n", msg_type);
   if (msg_type == 0xfff) {
      msg_type = 0x1ff;
   }
   qCamera->disableMsgType(msg_type);
}

int
qcamera_msg_type_enabled(struct camera_device * device, int32_t msg_type)
{
   LOGV("qcamera_msg_type_enabled: msg_type:%d\n", msg_type);
   return qCamera->msgTypeEnabled(msg_type);
}

int
qcamera_start_preview(struct camera_device * device)
{
   LOGV("qcamera_start_preview: Enabling CAMERA_MSG_PREVIEW_FRAME\n");

   /* TODO: Remove hack. */
   LOGV("qcamera_start_preview: Preview enabled:%d msg enabled:%d\n",
        qCamera->previewEnabled(),
        qCamera->msgTypeEnabled(CAMERA_MSG_PREVIEW_FRAME));
   if (!qCamera->msgTypeEnabled(CAMERA_MSG_PREVIEW_FRAME)) {
      qCamera->enableMsgType(CAMERA_MSG_PREVIEW_FRAME);
   }
   return qCamera->startPreview();
}

void
qcamera_stop_preview(struct camera_device * device)
{
   LOGV("qcamera_stop_preview: msgenabled:%d\n",
        qCamera->msgTypeEnabled(CAMERA_MSG_PREVIEW_FRAME));

   /* TODO: Remove hack. */
   if (qCamera->msgTypeEnabled(CAMERA_MSG_PREVIEW_FRAME)) {
      qCamera->disableMsgType(CAMERA_MSG_PREVIEW_FRAME);
   }
   return qCamera->stopPreview();
}

int
qcamera_preview_enabled(struct camera_device * device)
{
   LOGV("qcamera_preview_enabled:\n");
   return qCamera->previewEnabled() ? 1 : 0;
}

int
qcamera_store_meta_data_in_buffers(struct camera_device * device, int enable)
{
   LOGV("qcamera_store_meta_data_in_buffers:\n");
   return NO_ERROR;
}

int
qcamera_start_recording(struct camera_device * device)
{
   LOGV("qcamera_start_recording\n");

   /* TODO: Remove hack. */
   qCamera->enableMsgType(CAMERA_MSG_VIDEO_FRAME);
   qCamera->startRecording();
   return NO_ERROR;
}

void
qcamera_stop_recording(struct camera_device * device)
{
   LOGV("qcamera_stop_recording:\n");

   /* TODO: Remove hack. */
   qCamera->disableMsgType(CAMERA_MSG_VIDEO_FRAME);
   qCamera->stopRecording();
}

int
qcamera_recording_enabled(struct camera_device * device)
{
   LOGV("qcamera_recording_enabled:\n");
   return (int)qCamera->recordingEnabled();
}

void
qcamera_release_recording_frame(struct camera_device * device,
                                const void *opaque)
{
   /* 
    * We release the frame immediately in CameraHAL_DataTSCb after making a
    * copy. So, this is just a NOP.
    */
   LOGV("qcamera_release_recording_frame: opaque:%p\n", opaque);
}

int
qcamera_auto_focus(struct camera_device * device)
{
   LOGV("qcamera_auto_focus:\n");
   qCamera->autoFocus();
   return NO_ERROR;
}

int
qcamera_cancel_auto_focus(struct camera_device * device)
{
   LOGV("qcamera_cancel_auto_focus:\n");
   qCamera->cancelAutoFocus();
   return NO_ERROR;
}

int
qcamera_take_picture(struct camera_device * device)
{
   LOGV("qcamera_take_picture:\n");

   /* TODO: Remove hack. */
   qCamera->enableMsgType(CAMERA_MSG_SHUTTER |
                         CAMERA_MSG_POSTVIEW_FRAME |
                         CAMERA_MSG_RAW_IMAGE |
                         CAMERA_MSG_COMPRESSED_IMAGE);

   qCamera->takePicture();
   return NO_ERROR;
}

int
qcamera_cancel_picture(struct camera_device * device)
{
   LOGV("camera_cancel_picture:\n");
   qCamera->cancelPicture();
   return NO_ERROR;
}

int
qcamera_set_parameters(struct camera_device * device, const char *params)
{
   LOGV("qcamera_set_parameters: %s\n", params);
   g_str = android::String8(params);
   camSettings.unflatten(g_str);
   qCamera->setParameters(camSettings);
   return NO_ERROR;
}

char*
qcamera_get_parameters(struct camera_device * device)
{
   char *rc = NULL;
   LOGV("qcamera_get_parameters\n");
   camSettings = qCamera->getParameters();
   LOGV("qcamera_get_parameters: after calling qCamera->getParameters()\n");
   CameraHAL_FixupParams(camSettings);
   g_str = camSettings.flatten();
   rc = strdup((char *)g_str.string());
   LOGV("camera_get_parameters: returning rc:%p :%s\n",
        rc, (rc != NULL) ? rc : "EMPTY STRING");
   return rc;
}

void
qcamera_put_parameters(struct camera_device *device, char *params)
{
   LOGV("qcamera_put_parameters: params:%p %s", params, params);
   free(params);
}


int
qcamera_send_command(struct camera_device * device, int32_t cmd, 
                        int32_t arg0, int32_t arg1)
{
   LOGV("qcamera_send_command: cmd:%d arg0:%d arg1:%d\n",
        cmd, arg0, arg1);
   return qCamera->sendCommand(cmd, arg0, arg1);
}

void
qcamera_release(struct camera_device * device)
{
   LOGV("camera_release:\n");
   qCamera->release();
}

int
qcamera_dump(struct camera_device * device, int fd)
{
   LOGV("qcamera_dump:\n");
   android::Vector<android::String16> args;
   return qCamera->dump(fd, args);
}

int
camera_device_close(hw_device_t* device)
{
   int rc = -EINVAL;
   LOGD("camera_device_close\n");
   camera_device_t *cameraDev = (camera_device_t *)device;
   if (cameraDev) {
      camera_device_ops_t *camera_ops = cameraDev->ops;
      if (camera_ops) {
         if (qCamera != NULL) {
            qCamera.clear();
         }
         free(camera_ops);
      }
      free(cameraDev);
      rc = NO_ERROR;
   }
   return rc;
}


int
qcamera_device_open(const hw_module_t* module, const char* name,
                   hw_device_t** device)
{

   void *libcameraHandle;
   int cameraId = atoi(name);

   LOGD("qcamera_device_open: name:%s device:%p cameraId:%d\n",
        name, device, cameraId);

   libcameraHandle = ::dlopen("libcamera.so", RTLD_NOW);
   LOGD("loading libcamera at %p", libcameraHandle);
   if (!libcameraHandle) {
       LOGE("FATAL ERROR: could not dlopen libcamera.so: %s", dlerror());
       return false;
   }

   if (::dlsym(libcameraHandle, "openCameraHardware") != NULL) {
      *(void**)&LINK_openCameraHardware =
               ::dlsym(libcameraHandle, "openCameraHardware");
   } else if (::dlsym(libcameraHandle, "HAL_openCameraHardware") != NULL) {
      *(void**)&LINK_openCameraHardware =
               ::dlsym(libcameraHandle, "HAL_openCameraHardware");
   } else {
      LOGE("FATAL ERROR: Could not find openCameraHardware");
      dlclose(libcameraHandle);
      return false;
   }

   qCamera = LINK_openCameraHardware(cameraId);
   ::dlclose(libcameraHandle);

   camera_device_t* camera_device = NULL;
   camera_device_ops_t* camera_ops = NULL;

   camera_device = (camera_device_t*)malloc(sizeof(*camera_device));
   camera_ops = (camera_device_ops_t*)malloc(sizeof(*camera_ops));
   memset(camera_device, 0, sizeof(*camera_device));
   memset(camera_ops, 0, sizeof(*camera_ops));

   camera_device->common.tag              = HARDWARE_DEVICE_TAG;
   camera_device->common.version          = 0;
   camera_device->common.module           = (hw_module_t *)(module);
   camera_device->common.close            = camera_device_close;
   camera_device->ops                     = camera_ops;

   camera_ops->set_preview_window         = qcamera_set_preview_window;
   camera_ops->set_callbacks              = qcamera_set_callbacks;
   camera_ops->enable_msg_type            = qcamera_enable_msg_type;
   camera_ops->disable_msg_type           = qcamera_disable_msg_type;
   camera_ops->msg_type_enabled           = qcamera_msg_type_enabled;
   camera_ops->start_preview              = qcamera_start_preview;
   camera_ops->stop_preview               = qcamera_stop_preview;
   camera_ops->preview_enabled            = qcamera_preview_enabled;
   camera_ops->store_meta_data_in_buffers = qcamera_store_meta_data_in_buffers;
   camera_ops->start_recording            = qcamera_start_recording;
   camera_ops->stop_recording             = qcamera_stop_recording;
   camera_ops->recording_enabled          = qcamera_recording_enabled;
   camera_ops->release_recording_frame    = qcamera_release_recording_frame;
   camera_ops->auto_focus                 = qcamera_auto_focus;
   camera_ops->cancel_auto_focus          = qcamera_cancel_auto_focus;
   camera_ops->take_picture               = qcamera_take_picture;
   camera_ops->cancel_picture             = qcamera_cancel_picture;

   camera_ops->set_parameters             = qcamera_set_parameters;
   camera_ops->get_parameters             = qcamera_get_parameters;
   camera_ops->put_parameters             = qcamera_put_parameters;
   camera_ops->send_command               = qcamera_send_command;
   camera_ops->release                    = qcamera_release;
   camera_ops->dump                       = qcamera_dump;

   *device = &camera_device->common;
   return NO_ERROR;
}
