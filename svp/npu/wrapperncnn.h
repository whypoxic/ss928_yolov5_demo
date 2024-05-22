#ifndef _WRAPPERNCNN_H__
#define _WRAPPERNCNN_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
JPG or PNG chang to yuv420sp W=640 scale
*/
int ncnn_convertimg_yolov5s(const char* jpg,const char* yuvpath);
int ncnn_result(const float *src,unsigned int len);

#ifdef __cplusplus
}
#endif
#endif