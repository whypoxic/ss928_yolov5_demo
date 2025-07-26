### 通过NCNN储存YUV420

~~~

    int imgin_w = 640,imgin_h = 640;
    cv::Mat a(imgin_w, imgin_h, CV_8UC3);
    memset(a.data,0xFF,imgin_w*imgin_h*3);
    in.to_pixels(a.data, ncnn::Mat::PIXEL_RGB2BGR);
    cv::imshow("in_image", a);

    //yuv420sp 
    ncnn::Mat yuv(imgin_w, imgin_h/2*3,1);
    unsigned char *puv =(unsigned char *) yuv+imgin_w*imgin_h;
    bgr2yuv420sp(a.data,imgin_w, imgin_h,yuv,puv,imgin_w);

    FILE* fp = fopen("output.yuv", "wb");
    if (fp) {
        fwrite(yuv, imgin_w*imgin_h*3/2,1, fp);
        fclose(fp);
    }
~~~

~~~

static void bgr2yuv420sp(const unsigned char* bgrdata, int width, int height, unsigned char* yptr, unsigned char* uvptr, int stride)
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
        const unsigned char* p0 = bgrdata + y * width * 3;
        const unsigned char* p1 = bgrdata + (y + 1) * width * 3;
        unsigned char* yptr0 = yptr + y * stride;
        unsigned char* yptr1 = yptr + (y + 1) * stride;
        unsigned char* uvptr0 = uvptr + (y / 2) * stride;

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

#define SATURATE_CAST_UCHAR(X) (unsigned char)::std::min(::std::max((int)(X), 0), 255);
            unsigned char y00 = SATURATE_CAST_UCHAR(( 38 * r00 + 75 * g00 +  15 * b00 + 64) >> 7);
            unsigned char y01 = SATURATE_CAST_UCHAR(( 38 * r01 + 75 * g01 +  15 * b01 + 64) >> 7);
            unsigned char y10 = SATURATE_CAST_UCHAR(( 38 * r10 + 75 * g10 +  15 * b10 + 64) >> 7);
            unsigned char y11 = SATURATE_CAST_UCHAR(( 38 * r11 + 75 * g11 +  15 * b11 + 64) >> 7);

            unsigned char b4 = (b00 + b01 + b10 + b11) / 4;
            unsigned char g4 = (g00 + g01 + g10 + g11) / 4;
            unsigned char r4 = (r00 + r01 + r10 + r11) / 4;

            // unsigned char b4 = b00;
            // unsigned char g4 = g00;
            // unsigned char r4 = r00;

            unsigned char u = SATURATE_CAST_UCHAR(((-43 * r4 -  84 * g4 + 127 * b4 + 128) >> 8) + 128);
            unsigned char v = SATURATE_CAST_UCHAR(((127 * r4 - 107 * g4 -  20 * b4 + 128) >> 8) + 128);
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
~~~