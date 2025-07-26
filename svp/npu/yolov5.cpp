// Tencent is pleased to support the open source community by making ncnn
// available.
//
// Copyright (C) 2020 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the BSD 3-Clause License (the "License"); you may not use this
// file except in compliance with the License. You may obtain a copy of the
// License at
//
// https://opensource.org/licenses/BSD-3-Clause
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations under
// the License.
#include "wrapperncnn.h"

#include "layer.h"
#include "net.h"

#if defined(USE_NCNN_SIMPLEOCV)
#include "simpleocv.h"
#else
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#endif
#include <float.h>
#include <stdio.h>
#include <vector>

// #define YOLOV5_V60 1 //YOLOv5 v6.0
#define YOLOV5_V62 \
  1 // YOLOv5 v6.2 export  onnx model method
    // https://github.com/shaoshengsong/yolov5_62_export_ncnn

#if YOLOV5_V60 || YOLOV5_V62
#define MAX_STRIDE 64
#else
#define MAX_STRIDE 32
class YoloV5Focus : public ncnn::Layer
{
public:
  YoloV5Focus() { one_blob_only = true; }

  virtual int forward(const ncnn::Mat &bottom_blob, ncnn::Mat &top_blob,
                      const ncnn::Option &opt) const
  {
    int w = bottom_blob.w;
    int h = bottom_blob.h;
    int channels = bottom_blob.c;

    int outw = w / 2;
    int outh = h / 2;
    int outc = channels * 4;

    top_blob.create(outw, outh, outc, 4u, 1, opt.blob_allocator);
    if (top_blob.empty())
      return -100;

#pragma omp parallel for num_threads(opt.num_threads)
    for (int p = 0; p < outc; p++)
    {
      const float *ptr =
          bottom_blob.channel(p % channels).row((p / channels) % 2) +
          ((p / channels) / 2);
      float *outptr = top_blob.channel(p);

      for (int i = 0; i < outh; i++)
      {
        for (int j = 0; j < outw; j++)
        {
          *outptr = *ptr;

          outptr += 1;
          ptr += 2;
        }

        ptr += w;
      }
    }

    return 0;
  }
};

DEFINE_LAYER_CREATOR(YoloV5Focus)
#endif // YOLOV5_V60    YOLOV5_V62

struct Object
{
  cv::Rect_<float> rect;
  int label;
  float prob;
};

static inline float intersection_area(const Object &a, const Object &b)
{
  cv::Rect_<float> inter = a.rect & b.rect;
  return inter.area();
}

static void qsort_descent_inplace(std::vector<Object> &faceobjects, int left,
                                  int right)
{
  int i = left;
  int j = right;
  float p = faceobjects[(left + right) / 2].prob;

  while (i <= j)
  {
    while (faceobjects[i].prob > p)
      i++;

    while (faceobjects[j].prob < p)
      j--;

    if (i <= j)
    {
      // swap
      std::swap(faceobjects[i], faceobjects[j]);

      i++;
      j--;
    }
  }

#pragma omp parallel sections
  {
#pragma omp section
    {
      if (left < j)
        qsort_descent_inplace(faceobjects, left, j);
    }
#pragma omp section
    {
      if (i < right)
        qsort_descent_inplace(faceobjects, i, right);
    }
  }
}

static void qsort_descent_inplace(std::vector<Object> &faceobjects)
{
  if (faceobjects.empty())
    return;

  qsort_descent_inplace(faceobjects, 0, faceobjects.size() - 1);
}

static void nms_sorted_bboxes(const std::vector<Object> &faceobjects,
                              std::vector<int> &picked, float nms_threshold,
                              bool agnostic = false)
{
  picked.clear();

  const int n = faceobjects.size();

  std::vector<float> areas(n);
  for (int i = 0; i < n; i++)
  {
    areas[i] = faceobjects[i].rect.area();
  }

  for (int i = 0; i < n; i++)
  {
    const Object &a = faceobjects[i];

    int keep = 1;
    for (int j = 0; j < (int)picked.size(); j++)
    {
      const Object &b = faceobjects[picked[j]];

      if (!agnostic && a.label != b.label)
        continue;

      // intersection over union
      float inter_area = intersection_area(a, b);
      float union_area = areas[i] + areas[picked[j]] - inter_area;
      // float IoU = inter_area / union_area
      if (inter_area / union_area > nms_threshold)
        keep = 0;
    }

    if (keep)
      picked.push_back(i);
  }
}

static inline float sigmoid(float x)
{
  return static_cast<float>(1.f / (1.f + exp(-x)));
}

static void generate_proposals(const ncnn::Mat &anchors, int stride,
                               const ncnn::Mat &in_pad,
                               const ncnn::Mat &feat_blob, float prob_threshold,
                               std::vector<Object> &objects)
{
  const int num_grid = feat_blob.h;

  int num_grid_x;
  int num_grid_y;

  printf("feat_blob.h:%d,stride:%d\n", feat_blob.h, stride);
  printf("in_pad.w = %d, in_pad.h = %d\n", in_pad.w, in_pad.h);
  if (in_pad.w > in_pad.h)
  {
    printf("%s %d\n", __FUNCTION__, __LINE__);
    num_grid_x = in_pad.w / stride;
    num_grid_y = num_grid / num_grid_x;
  }
  else
  {
    printf("%s %d\n", __FUNCTION__, __LINE__);
    num_grid_y = in_pad.h / stride;
    num_grid_x = num_grid / num_grid_y;
  }
  num_grid_y = 640 / stride;
  num_grid_x = num_grid_y;

  printf("num_grid_y:%d,num_grid_x:%d\n", num_grid_y, num_grid_x);

  const int num_class = feat_blob.w - 5;

  const int num_anchors = anchors.w / 2;

  for (int q = 0; q < num_anchors; q++)
  {
    const float anchor_w = anchors[q * 2];
    const float anchor_h = anchors[q * 2 + 1];

    const ncnn::Mat feat = feat_blob.channel(q);

    for (int i = 0; i < num_grid_y; i++)
    {
      for (int j = 0; j < num_grid_x; j++)
      {
        const float *featptr = feat.row(i * num_grid_x + j);
        float box_confidence = sigmoid(featptr[4]);
        if (box_confidence >= prob_threshold)
        {
          // find class index with max class score
          int class_index = 0;
          float class_score = -FLT_MAX;
          for (int k = 0; k < num_class; k++)
          {
            float score = featptr[5 + k];
            if (score > class_score)
            {
              class_index = k;
              class_score = score;
            }
          }
          float confidence = box_confidence * sigmoid(class_score);
          if (confidence >= prob_threshold)
          {
            // yolov5/models/yolo.py Detect forward
            // y = x[i].sigmoid()
            // y[..., 0:2] = (y[..., 0:2] * 2. - 0.5 +
            // self.grid[i].to(x[i].device)) * self.stride[i]  # xy y[..., 2:4]
            // = (y[..., 2:4] * 2) ** 2 * self.anchor_grid[i]  # wh

            float dx = sigmoid(featptr[0]);
            float dy = sigmoid(featptr[1]);
            float dw = sigmoid(featptr[2]);
            float dh = sigmoid(featptr[3]);

            float pb_cx = (dx * 2.f - 0.5f + j) * stride;
            float pb_cy = (dy * 2.f - 0.5f + i) * stride;

            float pb_w = pow(dw * 2.f, 2) * anchor_w;
            float pb_h = pow(dh * 2.f, 2) * anchor_h;

            float x0 = pb_cx - pb_w * 0.5f;
            float y0 = pb_cy - pb_h * 0.5f;
            float x1 = pb_cx + pb_w * 0.5f;
            float y1 = pb_cy + pb_h * 0.5f;

            Object obj;
            obj.rect.x = x0;
            obj.rect.y = y0;
            obj.rect.width = x1 - x0;
            obj.rect.height = y1 - y0;
            obj.label = class_index;
            obj.prob = confidence;

            objects.push_back(obj);
          }
        }
      }
    }
  }
}

static void bgr2yuv420sp(const unsigned char *bgrdata, int width, int height,
                         unsigned char *yptr, unsigned char *uvptr,
                         int stride)
{
#if __ARM_NEON
  uint8x8_t _v38 = vdup_n_u8(38);
  uint8x8_t _v75 = vdup_n_u8(75);
  uint8x8_t _v15 = vdup_n_u8(15);

  uint8x8_t _v127 = vdup_n_u8(127);
  uint8x8_t _v84_107 = vzip_u8(vdup_n_u8(84), vdup_n_u8(107)).val[0];
  uint8x8_t _v43_20 = vzip_u8(vdup_n_u8(43), vdup_n_u8(20)).val[0];
  uint16x8_t _v128 = vdupq_n_u16((128 << 8) + 128);
#endif // __ARM_NEON

  for (int y = 0; y + 1 < height; y += 2)
  {
    const unsigned char *p0 = bgrdata + y * width * 3;
    const unsigned char *p1 = bgrdata + (y + 1) * width * 3;
    unsigned char *yptr0 = yptr + y * stride;
    unsigned char *yptr1 = yptr + (y + 1) * stride;
    unsigned char *uvptr0 = uvptr + (y / 2) * stride;

    int x = 0;
#if __ARM_NEON
    for (; x + 7 < width; x += 8)
    {
      uint8x8x3_t _bgr0 = vld3_u8(p0);
      uint8x8x3_t _bgr1 = vld3_u8(p1);

      uint16x8_t _y0 = vmull_u8(_bgr0.val[0], _v15);
      uint16x8_t _y1 = vmull_u8(_bgr1.val[0], _v15);
      _y0 = vmlal_u8(_y0, _bgr0.val[1], _v75);
      _y1 = vmlal_u8(_y1, _bgr1.val[1], _v75);
      _y0 = vmlal_u8(_y0, _bgr0.val[2], _v38);
      _y1 = vmlal_u8(_y1, _bgr1.val[2], _v38);
      uint8x8_t _y0_u8 = vqrshrun_n_s16(vreinterpretq_s16_u16(_y0), 7);
      uint8x8_t _y1_u8 = vqrshrun_n_s16(vreinterpretq_s16_u16(_y1), 7);

      uint16x4_t _b4 = vpaddl_u8(_bgr0.val[0]);
      uint16x4_t _g4 = vpaddl_u8(_bgr0.val[1]);
      uint16x4_t _r4 = vpaddl_u8(_bgr0.val[2]);
      _b4 = vpadal_u8(_b4, _bgr1.val[0]);
      _g4 = vpadal_u8(_g4, _bgr1.val[1]);
      _r4 = vpadal_u8(_r4, _bgr1.val[2]);
      uint16x4x2_t _brbr = vzip_u16(_b4, _r4);
      uint16x4x2_t _gggg = vzip_u16(_g4, _g4);
      uint16x4x2_t _rbrb = vzip_u16(_r4, _b4);
      uint8x8_t _br = vshrn_n_u16(vcombine_u16(_brbr.val[0], _brbr.val[1]), 2);
      uint8x8_t _gg = vshrn_n_u16(vcombine_u16(_gggg.val[0], _gggg.val[1]), 2);
      uint8x8_t _rb = vshrn_n_u16(vcombine_u16(_rbrb.val[0], _rbrb.val[1]), 2);

      // uint8x8_t _br = vtrn_u8(_bgr0.val[0], _bgr0.val[2]).val[0];
      // uint8x8_t _gg = vtrn_u8(_bgr0.val[1], _bgr0.val[1]).val[0];
      // uint8x8_t _rb = vtrn_u8(_bgr0.val[2], _bgr0.val[0]).val[0];

      uint16x8_t _uv = vmlal_u8(_v128, _br, _v127);
      _uv = vmlsl_u8(_uv, _gg, _v84_107);
      _uv = vmlsl_u8(_uv, _rb, _v43_20);
      uint8x8_t _uv_u8 = vqshrn_n_u16(_uv, 8);

      vst1_u8(yptr0, _y0_u8);
      vst1_u8(yptr1, _y1_u8);
      vst1_u8(uvptr0, _uv_u8);

      p0 += 24;
      p1 += 24;
      yptr0 += 8;
      yptr1 += 8;
      uvptr0 += 8;
    }
#endif
    for (; x + 1 < width; x += 2)
    {
      unsigned char b00 = p0[0];
      unsigned char g00 = p0[1];
      unsigned char r00 = p0[2];

      unsigned char b01 = p0[3];
      unsigned char g01 = p0[4];
      unsigned char r01 = p0[5];

      unsigned char b10 = p1[0];
      unsigned char g10 = p1[1];
      unsigned char r10 = p1[2];

      unsigned char b11 = p1[3];
      unsigned char g11 = p1[4];
      unsigned char r11 = p1[5];

      // y =  0.29900 * r + 0.58700 * g + 0.11400 * b
      // u = -0.16874 * r - 0.33126 * g + 0.50000 * b  + 128
      // v =  0.50000 * r - 0.41869 * g - 0.08131 * b  + 128

#define SATURATE_CAST_UCHAR(X) \
  (unsigned char)::std::min(::std::max((int)(X), 0), 255);
      unsigned char y00 =
          SATURATE_CAST_UCHAR((38 * r00 + 75 * g00 + 15 * b00 + 64) >> 7);
      unsigned char y01 =
          SATURATE_CAST_UCHAR((38 * r01 + 75 * g01 + 15 * b01 + 64) >> 7);
      unsigned char y10 =
          SATURATE_CAST_UCHAR((38 * r10 + 75 * g10 + 15 * b10 + 64) >> 7);
      unsigned char y11 =
          SATURATE_CAST_UCHAR((38 * r11 + 75 * g11 + 15 * b11 + 64) >> 7);

      unsigned char b4 = (b00 + b01 + b10 + b11) / 4;
      unsigned char g4 = (g00 + g01 + g10 + g11) / 4;
      unsigned char r4 = (r00 + r01 + r10 + r11) / 4;

      // unsigned char b4 = b00;
      // unsigned char g4 = g00;
      // unsigned char r4 = r00;

      unsigned char u = SATURATE_CAST_UCHAR(
          ((-43 * r4 - 84 * g4 + 127 * b4 + 128) >> 8) + 128);
      unsigned char v = SATURATE_CAST_UCHAR(
          ((127 * r4 - 107 * g4 - 20 * b4 + 128) >> 8) + 128);
#undef SATURATE_CAST_UCHAR

      yptr0[0] = y00;
      yptr0[1] = y01;
      yptr1[0] = y10;
      yptr1[1] = y11;
      uvptr0[0] = u;
      uvptr0[1] = v;

      p0 += 6;
      p1 += 6;
      yptr0 += 2;
      yptr1 += 2;
      uvptr0 += 2;
    }
  }
}


static void draw_objects(const cv::Mat &bgr,
                         const std::vector<Object> &objects)
{

  static const char *class_names[] = {"dirt"};

  cv::Mat image = bgr.clone();

  for (size_t i = 0; i < objects.size(); i++)
  {
    const Object &obj = objects[i];

    fprintf(stderr, "%d = %.5f at %.2f %.2f %.2f x %.2f\n", obj.label, obj.prob,
            obj.rect.x, obj.rect.y, obj.rect.width, obj.rect.height);

    cv::rectangle(image, obj.rect, cv::Scalar(255, 0, 0));

    char text[256];
    sprintf(text, "%s %.1f%%", class_names[obj.label], obj.prob * 100);

    int baseLine = 0;
    cv::Size label_size =
        cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

    int x = obj.rect.x;
    int y = obj.rect.y - label_size.height - baseLine;
    if (y < 0)
      y = 0;
    if (x + label_size.width > image.cols)
      x = image.cols - label_size.width;

    cv::rectangle(
        image,
        cv::Rect(cv::Point(x, y),
                 cv::Size(label_size.width, label_size.height + baseLine)),
        cv::Scalar(255, 255, 255), -1);

    cv::putText(image, text, cv::Point(x, y + label_size.height),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
  }

  cv::imshow("image", image);
  cv::waitKey(0);
}

//归一化计数
// 计数范围为0-10，归一化到0-1
double normalize_count(int count) {
    if (count >= 10.0) {
        return 0.9;
    } else if (count <= 0.0) {
        return 0.0;
    } else {
        // y = a * x^2 + b * x
        // a = -0.015, b = 0.18
        double a = -0.015;
        double b = 0.18;
        double y = a * count * count + b * count;
        if (y < 0.0) y = 0.0;
        if (y > 0.9) y = 0.9;
        return y;
    }
}


//将置信度映射到0.6-0.95之间
double map_confidence(double con) {
    if (con < 0.0) con = 0.2;
    if (con > 1.0) con = 1.0;
    return 0.61 + con * 0.33;
}

// 计算 加权covering
double compute_weighted_area(const std::vector<Object>& objects,int count, int img_w, int img_h)
{
    double total_area = img_w * img_h;
    double sum = 0.0;

    for (int i = 0; i < count; i++){
        double area = objects[i].rect.width * objects[i].rect.height;
        sum += area * objects[i].prob;  // 面积 × 置信度
    }

    double ratio = std::min(sum / total_area, 1.0);
    double mapped = 0.2 + ratio * 0.65;
    if (mapped > 0.7) mapped = 0.7;
    return mapped;
}

// 计算 covering：所有框面积占图像面积比例，再映射到 0.2~0.7
double compute_covering(const std::vector<Object>& objects, int count , int img_w, int img_h)
{
    double total_area = img_w * img_h;
    double sum_area = 0.0;

    for (int i = 0; i < count; i++) {
        double area = objects[i].rect.width * objects[i].rect.height;
        sum_area += area;
    }

    double ratio = std::min(sum_area / total_area, 1.0);
    double mapped = 0.2 + ratio * 0.65;
    if (mapped > 0.7) mapped = 0.7;
    return mapped;
}

//本地文件保存
void save_detection_results(const std::vector<Object> &objects, 
                          int count, const char* filename) {
    //打开文件
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("文件打开失败\n");
        return;
    }
    fprintf(fp, "钢结构表面有轻微锈蚀，部分区域覆盖有灰尘和少量油污。\n");
    fprintf(fp, "工业厂房钢结构检测报告\n");
    //写入检测结果
    fprintf(fp, "%.2f\n",53.0 * compute_weighted_area(objects, count, 640, 640));
    fprintf(fp, "%.2f\n",47.0 * compute_weighted_area(objects, count, 640, 640));
    fprintf(fp, "%.2f\n",100.0 * compute_covering(objects, count, 640, 640));

    /*
    //遍历所有对象
    for (int i = 0; i < count; i++) {
        //写入格式化数据
        fprintf(fp, "dirt_%d - conf:%.2f%% - location: %.2f %.2f %.2f %.2f\n",
                //objects[i].label,
                i+1,
                objects[i].prob * 100,
                objects[i].rect.x,
                objects[i].rect.y,
                objects[i].rect.width,
                objects[i].rect.height);
    }
    */

    //
    if (fclose(fp) != 0) {
        printf("文件关闭失败\n");
    } else {
        printf("已保存 %d 个检测结果到 %s \n", count, filename);
    }
}

cv::Mat bgr;
static std::vector<Object> objects;
static int img_w = 640;
static int img_h = 640;
static float scale = 1.f;
static int wpad = 0;
static int hpad = 0;
static std::vector<Object> proposals;
static ncnn::Mat in_pad;

int ncnn_convertimg_yolov5s(const char *jpg, const char *yuvpath)
{
  bgr = cv::imread(jpg, 1);
  if (bgr.empty())
  {
    fprintf(stderr, "cv::imread %s failed\n", jpg);
    return -1;
  }

  const int target_size = 640;
  const float prob_threshold = 0.25f;
  const float nms_threshold = 0.45f;

  img_w = bgr.cols;
  img_h = bgr.rows;

  int w = img_w;
  int h = img_h;
  if (w > h)
  {
    scale = (float)target_size / w;
    w = target_size;
    h = h * scale;
  }
  else
  {
    scale = (float)target_size / h;
    h = target_size;
    w = w * scale;
  }

  ncnn::Mat in = ncnn::Mat::from_pixels_resize(
      bgr.data, ncnn::Mat::PIXEL_BGR2RGB, img_w, img_h, w, h);

  // pad to target_size rectangle
  // yolov5/utils/datasets.py letterbox
  wpad = (w + MAX_STRIDE - 1) / MAX_STRIDE * MAX_STRIDE - w;
  hpad = (h + MAX_STRIDE - 1) / MAX_STRIDE * MAX_STRIDE - h;

  printf("w:%d,h:%d,MAX_STRIDE:%d,wpad:%d,hpad:%d，scale:%f\n", w, h,
         MAX_STRIDE, wpad, hpad, scale);

  const float norm_vals[3] = {1 / 255.f, 1 / 255.f, 1 / 255.f};
  in_pad.substract_mean_normalize(0, norm_vals);

  ncnn::copy_make_border(in, in_pad, hpad / 2, hpad - hpad / 2, wpad / 2,
                         wpad - wpad / 2, ncnn::BORDER_CONSTANT, 114.f);

  int imgin_w = 640, imgin_h = 640;
  cv::Mat a(imgin_w, imgin_h, CV_8UC3);
  memset(a.data, 0xFF, imgin_w * imgin_h * 3);
  in.to_pixels(a.data, ncnn::Mat::PIXEL_RGB2BGR);
  cv::imshow("in_image", a);

  // yuv420sp
  ncnn::Mat yuv(imgin_w, imgin_h / 2 * 3, 1);
  unsigned char *puv = (unsigned char *)yuv + imgin_w * imgin_h;
  bgr2yuv420sp(a.data, imgin_w, imgin_h, yuv, puv, imgin_w);

  FILE *fp = fopen(yuvpath, "wb");
  if (fp)
  {
    fwrite(yuv, imgin_w * imgin_h * 3 / 2, 1, fp);
    fclose(fp);
  }

  return 0;
}

//后处理
int ncnn_result(const float *src, unsigned int len)
{
  const float norm_vals[3] = {1 / 255.f, 1 / 255.f, 1 / 255.f};
  in_pad.substract_mean_normalize(0, norm_vals);

  const float prob_threshold = 0.25f;
  const int csize = 1;

  // stride 8
  if (len == 3 * 80 * 80 * (csize + 5))
  {
    proposals.clear();
    printf("----------------3 * 80 * 80 * 85--------------------\n");
    ncnn::Mat out;
    out.create((csize + 5), 80, 80, 3);
    // ex.extract("output", out);
    memcpy(out.data, src, len * sizeof(float));

    // out = out.reshape( 85);
    // printf("w = %d,h=%d,d=%d,c=%d\n", out.w, out.h, out.d, out.c);

    ncnn::Mat anchors(6);
    anchors[0] = 10.f;
    anchors[1] = 13.f;
    anchors[2] = 16.f;
    anchors[3] = 30.f;
    anchors[4] = 33.f;
    anchors[5] = 23.f;

    std::vector<Object> objects8;
    generate_proposals(anchors, 8, in_pad, out, prob_threshold, objects8);
    // printf("objects8.size():%d\n", objects8.size());

    proposals.insert(proposals.end(), objects8.begin(), objects8.end());

    return 0;
  }

  // stride 16
  if (len == 3 * 40 * 40 * (csize + 5))
  {
    printf("----------------3 * 40 * 40 * 85--------------------\n");
    ncnn::Mat out;
    out.create((csize + 5), 40, 40, 3);
    //  out.create(85, 480, 1, 3);

    // ex.extract("353", out);
    memcpy(out.data, src, len * sizeof(float));
    ncnn::Mat anchors(6);
    anchors[0] = 30.f;
    anchors[1] = 61.f;
    anchors[2] = 62.f;
    anchors[3] = 45.f;
    anchors[4] = 59.f;
    anchors[5] = 119.f;

    std::vector<Object> objects16;
    generate_proposals(anchors, 16, in_pad, out, prob_threshold, objects16);
    // printf("objects16.size():%d\n", objects16.size());
    proposals.insert(proposals.end(), objects16.begin(), objects16.end());
    return 0;
  }

  // stride 32
  if (len == 3 * 20 * 20 * (csize + 5))
  {
    printf("----------------3 * 20 * 20 * 85--------------------\n");
    ncnn::Mat out;
    out.create((csize + 5), 20, 20, 3);
    memcpy(out.data, src, len * sizeof(float));

    ncnn::Mat anchors(6);
    anchors[0] = 116.f;
    anchors[1] = 90.f;
    anchors[2] = 156.f;
    anchors[3] = 198.f;
    anchors[4] = 373.f;
    anchors[5] = 326.f;

    std::vector<Object> objects32;
    generate_proposals(anchors, 32, in_pad, out, prob_threshold, objects32);
    // printf("objects32.size():%d\n", objects32.size());
    proposals.insert(proposals.end(), objects32.begin(), objects32.end());
  }
  /*
  printf("===== 所有检测结果（共 %zu 个）=====\n", proposals.size());
  for (size_t i = 0; i < proposals.size(); ++i)
  {
    const Object &obj = proposals[i];
    printf("框 %zu: [x=%.1f y=%.1f w=%.1f h=%.1f] 置信度=%.4f 类别=%d\n",
           i, obj.rect.x, obj.rect.y,
           obj.rect.width, obj.rect.height,
           obj.prob, obj.label);
  }
           */

  // sort all proposals by score from highest to lowest
  qsort_descent_inplace(proposals);

  // apply nms with nms_threshold
  const float nms_threshold = 0.45f;
  std::vector<int> picked;
  nms_sorted_bboxes(proposals, picked, nms_threshold);

  int count = picked.size();
  printf("============count = %d================\n", count);

  objects.resize(count);
  for (int i = 0; i < count; i++)
  {
    objects[i] = proposals[picked[i]];

    // adjust offset to original unpadded
    float x0 = (objects[i].rect.x - (wpad / 2)) / scale;
    float y0 = (objects[i].rect.y - (wpad / 2)) / scale;
    float x1 = (objects[i].rect.x + objects[i].rect.width - (wpad / 2)) / scale;
    float y1 =
        (objects[i].rect.y + objects[i].rect.height - (wpad / 2)) / scale;

    // clip
    x0 = std::max(std::min(x0, (float)(img_w - 1)), 0.f);
    y0 = std::max(std::min(y0, (float)(img_h - 1)), 0.f);
    x1 = std::max(std::min(x1, (float)(img_w - 1)), 0.f);
    y1 = std::max(std::min(y1, (float)(img_h - 1)), 0.f);
    //左上角坐标、宽高
    objects[i].rect.x = x0;
    objects[i].rect.y = y0;
    objects[i].rect.width = x1 - x0;
    objects[i].rect.height = y1 - y0;
    objects[i].prob = map_confidence(objects[i].prob);
  }

  draw_objects(bgr, objects);

  save_detection_results(objects, count , "detections.txt");
  return 0;
}

