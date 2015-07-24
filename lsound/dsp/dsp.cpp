/**
@file dsp.cpp
@author t-sakai
@date 2015/07/08 create
*/
#include "dsp.h"
#include <emmintrin.h>

namespace lsound
{
    //----------------------------------------------------------------------------
    //---
    //--- Convert Number of Channels
    //---
    //----------------------------------------------------------------------------
    void conv_Short1ToShort2(void* dst, const void* s, s32 numSamples)
    {
        LSshort* d = reinterpret_cast<LSshort*>(dst);
        const LSshort* src = reinterpret_cast<const LSshort*>(s);

        s32 num = numSamples >> 3; //8個のshortをまとめて処理
        s32 offset = num << 3;
        s32 rem = numSamples - offset;

        const LSshort* p = src;
        LSshort* q = d;
        for(s32 i=0; i<num; ++i){
            __m128i t = _mm_loadu_si128((const __m128i*)p);
            __m128i r0 = _mm_unpackhi_epi16(t, t);
            __m128i r1 = _mm_unpacklo_epi16(t, t);
            _mm_storeu_si128((__m128i*)(q+0), r1);
            _mm_storeu_si128((__m128i*)(q+8), r0);
            p += 8;
            q += 16;
        }

        for(s32 i=0; i<rem; ++i){
            s32 j = i<<1;
            q[j+0] = p[i];
            q[j+1] = p[i];
        }

        //check
        d = reinterpret_cast<LSshort*>(dst);
        for(s32 i=0; i<numSamples; ++i){
            if(d[2*i+0] != src[i]
                || d[2*i+1] != src[i])
            {
                LASSERT(false);
            }
        }
    }

    void conv_Short2ToShort1(void* dst, const void* s, s32 numSamples)
    {
        LSshort* d = reinterpret_cast<LSshort*>(dst);
        const LSshort* src = reinterpret_cast<const LSshort*>(s);

        s32 num = numSamples >> 2; //8個のshortをまとめて処理
        s32 offset = num << 2;
        s32 rem = numSamples - offset;

        const __m128i izero = _mm_setzero_si128();
        __declspec(align(16)) LSshort tmp[8];

        const LSshort* p = src;
        LSshort* q = d;
        for(s32 i=0; i<num; ++i){
            //32bit整数r0, r1に変換
            __m128i t0 = _mm_loadu_si128((const __m128i*)p);
            __m128i t1 = _mm_cmpgt_epi16(izero, t0);
            __m128i r0 = _mm_unpackhi_epi16(t0, t1);
            __m128i r1 = _mm_unpacklo_epi16(t0, t1);

            __m128i r2 = _mm_add_epi32(r0, _mm_shuffle_epi32(r0, _MM_SHUFFLE(2, 3, 0, 1)));
            __m128i r3 = _mm_add_epi32(r1, _mm_shuffle_epi32(r1, _MM_SHUFFLE(2, 3, 0, 1)));

            r2 = _mm_srai_epi32(r2, 1);
            r3 = _mm_srai_epi32(r3, 1);
            __m128i r4 = _mm_packs_epi32(r3, r2);
            _mm_store_si128((__m128i*)tmp, r4);
            q[0] = tmp[0];
            q[1] = tmp[2];
            q[2] = tmp[4];
            q[3] = tmp[6];
            p += 8;
            q += 4;
        }

        for(s32 i=0; i<rem; ++i){
            s32 j = i<<1;
            s32 t = (p[j+0] + p[j+1]) >> 1;
            q[i] = static_cast<LSshort>(t);
        }
    }

    void conv_Float1ToFloat2(void* dst, const void* s, s32 numSamples)
    {
        LSfloat* d = reinterpret_cast<LSfloat*>(dst);
        const LSfloat* src = reinterpret_cast<const LSfloat*>(s);

        s32 num = numSamples >> 2; //4個のfloatをまとめて処理
        s32 offset = num << 2;
        s32 rem = numSamples - offset;

        const LSfloat* p = src;
        LSfloat* q = d;
        for(s32 i=0; i<num; ++i){
            __m128 f32_0 = _mm_loadu_ps(p);
            __m128 f32_1 = _mm_shuffle_ps(f32_0, f32_0, _MM_SHUFFLE(1, 1, 0, 0));
            __m128 f32_2 = _mm_shuffle_ps(f32_0, f32_0, _MM_SHUFFLE(3, 3, 2, 2));

            _mm_storeu_ps((q+0), f32_1);
            _mm_storeu_ps((q+4), f32_2);
            p += 4;
            q += 8;
        }

        for(s32 i=0; i<rem; ++i){
            s32 j=i<<1;
            q[j+0] = q[j+1] = p[i];
        }
    }

    void conv_Float2ToFloat1(void* dst, const void* s, s32 numSamples)
    {
        LSfloat* d = reinterpret_cast<LSfloat*>(dst);
        const LSfloat* src = reinterpret_cast<const LSfloat*>(s);

        s32 num = numSamples >> 2; //4個のfloatをまとめて処理
        s32 offset = num << 2;
        s32 rem = numSamples - offset;
        __m128 coff = _mm_set1_ps(0.5f);

        const LSfloat* p = src;
        LSfloat* q = d;
        for(s32 i=0; i<num; ++i){
            __m128 f32_0 = _mm_loadu_ps(p);
            __m128 f32_1 = _mm_shuffle_ps(f32_0, f32_0, _MM_SHUFFLE(2, 3, 1, 0));
            __m128 f32_2 = _mm_mul_ps(_mm_add_ps(f32_0, f32_1), coff);
            __m128 f32_3 = _mm_shuffle_ps(f32_2, f32_2, _MM_SHUFFLE(2, 0, 2, 0));

            _mm_storel_pi((__m64*)q, f32_3);
            p += 4;
            q += 2;
        }

        for(s32 i=0; i<rem; ++i){
            s32 j=i<<1;
            q[i] = 0.5f*(p[j+0]+p[j+1]);
        }
    }

    //----------------------------------------------------------------------------
    //---
    //--- Convert Type and Channels
    //---
    //----------------------------------------------------------------------------
    void conv_Short1ToByte1(void* dst, const void* s, s32 numSamples)
    {
        LSbyte* d = reinterpret_cast<LSbyte*>(dst);
        const LSshort* src = reinterpret_cast<const LSshort*>(s);

        for(s32 i=0; i<numSamples; ++i){
            d[i] = toByte(src[i]);
        }
    }

    void conv_Short2ToByte2(void* dst, const void* s, s32 numSamples)
    {
        conv_Short1ToByte1(dst, s, numSamples<<1);
    }

    void conv_Short1ToByte2(void* dst, const void* s, s32 numSamples)
    {
        LSbyte* d = reinterpret_cast<LSbyte*>(dst);
        const LSshort* src = reinterpret_cast<const LSshort*>(s);

        for(s32 i=0; i<numSamples; ++i){
            s32 j = i<<1;
            d[j+0] = d[j+1] = toByte(src[i]);
        }
    }

    void conv_Short2ToByte1(void* dst, const void* s, s32 numSamples)
    {
        LSbyte* d = reinterpret_cast<LSbyte*>(dst);
        const LSshort* src = reinterpret_cast<const LSshort*>(s);

        for(s32 i=0; i<numSamples; ++i){
            s32 j = i<<1;
            f32 v = 0.5f*(toFloat(src[j+0]) + toFloat(src[j+1]));
            d[i] = toByte(v);
        }
    }

    //----------------------------------------------------------------------------
    void conv_Short1ToFloat1(void* dst, const void* s, s32 numSamples)
    {
        LSfloat* d = reinterpret_cast<LSfloat*>(dst);
        const LSshort* src = reinterpret_cast<const LSshort*>(s);

        s32 num = numSamples >> 3; //8個のshortをまとめて処理
        s32 offset = num << 3;
        s32 rem = numSamples - offset;

        const __m128i izero = _mm_setzero_si128();
        const __m128 fcoff = _mm_set1_ps(1.0f/32767.0f);

        const LSshort* p = src;
        LSfloat* q = d;
        for(s32 i=0; i<num; ++i){
            //32bit浮動小数点r0, r1に変換
            __m128i t0 = _mm_loadu_si128((const __m128i*)p);
            __m128i t1 = _mm_cmpgt_epi16(izero, t0);
            __m128 r0 = _mm_cvtepi32_ps(_mm_unpackhi_epi16(t0, t1));
            __m128 r1 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(t0, t1));

            r0 = _mm_mul_ps(r0, fcoff);
            r1 = _mm_mul_ps(r1, fcoff);
            _mm_storeu_ps((q+0), r1);
            _mm_storeu_ps((q+4), r0);
            p += 8;
            q += 8;
        }

        for(s32 i=0; i<rem; ++i){
            q[i] = toFloat(p[i]);
        }
    }

    void conv_Short2ToFloat2(void* dst, const void* s, s32 numSamples)
    {
        //ステレオ分2倍
        conv_Short1ToFloat1(dst, s, numSamples<<1);
    }

    void conv_Short1ToFloat2(void* dst, const void* s, s32 numSamples)
    {
        LSfloat* d = reinterpret_cast<LSfloat*>(dst);
        const LSshort* src = reinterpret_cast<const LSshort*>(s);

        s32 num = numSamples >> 3; //8個のshortをまとめて処理
        s32 offset = num << 3;
        s32 rem = numSamples - offset;

        const __m128i izero = _mm_setzero_si128();
        const __m128 fcoff = _mm_set1_ps(1.0f/32767.0f);

        const LSshort* p = src;
        LSfloat* q = d;
        for(s32 i=0; i<num; ++i){
            __m128i t = _mm_loadu_si128((const __m128i*)p);
            __m128i s16_0 = _mm_unpackhi_epi16(t, t);
            __m128i s16_1 = _mm_unpacklo_epi16(t, t);

            __m128i t1 = _mm_cmpgt_epi16(izero, s16_0);
            __m128i t2 = _mm_cmpgt_epi16(izero, s16_1);

            __m128i s32_0 = _mm_unpackhi_epi16(s16_0, t1);
            __m128i s32_1 = _mm_unpacklo_epi16(s16_0, t1);
            __m128i s32_2 = _mm_unpackhi_epi16(s16_1, t2);
            __m128i s32_3 = _mm_unpacklo_epi16(s16_1, t2);

            //32bit浮動小数点に変換
            __m128 f32_0 = _mm_mul_ps(_mm_cvtepi32_ps(s32_0), fcoff);
            __m128 f32_1 = _mm_mul_ps(_mm_cvtepi32_ps(s32_1), fcoff);
            __m128 f32_2 = _mm_mul_ps(_mm_cvtepi32_ps(s32_2), fcoff);
            __m128 f32_3 = _mm_mul_ps(_mm_cvtepi32_ps(s32_3), fcoff);

            _mm_storeu_ps((q+0), f32_3);
            _mm_storeu_ps((q+4), f32_2);
            _mm_storeu_ps((q+8), f32_1);
            _mm_storeu_ps((q+12), f32_0);

            p += 8;
            q += 16;
        }

        for(s32 i=0; i<rem; ++i){
            s32 j = i<<1;
            q[j+0] = toFloat(p[i]);
            q[j+1] = toFloat(p[i]);
        }
    }

    void conv_Short2ToFloat1(void* dst, const void* s, s32 numSamples)
    {
        LSfloat* d = reinterpret_cast<LSfloat*>(dst);
        const LSshort* src = reinterpret_cast<const LSshort*>(s);

        s32 num = numSamples >> 2; //8個のshortをまとめて処理
        s32 offset = num << 2;
        s32 rem = numSamples - offset;

        const __m128i izero = _mm_setzero_si128();
        const __m128 fcoff = _mm_set1_ps(0.5f/32767.0f); //half

        const LSshort* p = src;
        LSfloat* q = d;
        for(s32 i=0; i<num; ++i){
            //32bit整数に変換
            __m128i t0 = _mm_loadu_si128((const __m128i*)p);
            __m128i t1 = _mm_cmpgt_epi16(izero, t0);
            __m128i s32_0 = _mm_unpackhi_epi16(t0, t1);
            __m128i s32_1 = _mm_unpacklo_epi16(t0, t1);

            //32bit浮動小数点に変換
            __m128 f32_0 = _mm_mul_ps(_mm_cvtepi32_ps(s32_0), fcoff);
            __m128 f32_1 = _mm_mul_ps(_mm_cvtepi32_ps(s32_1), fcoff);

            __m128 f32_2 = _mm_add_ps(f32_0, _mm_shuffle_ps(f32_0, f32_0, _MM_SHUFFLE(2, 3, 0, 1)));
            __m128 f32_3 = _mm_add_ps(f32_1, _mm_shuffle_ps(f32_1, f32_1, _MM_SHUFFLE(2, 3, 0, 1)));
            __m128 r = _mm_shuffle_ps(f32_3, f32_2, _MM_SHUFFLE(2, 0, 2, 0));

            _mm_storeu_ps(q, r);
            p += 8;
            q += 4;
        }

        for(s32 i=0; i<rem; ++i){
            s32 j = i<<1;
            q[i] = 0.5f*(toFloat(p[j+0]) + toFloat(p[j+1]));
        }
    }


    //----------------------------------------------------------------------------
    void conv_Float1ToByte1(void* dst, const void* s, s32 numSamples)
    {
        LSbyte* d = reinterpret_cast<LSbyte*>(dst);
        const LSfloat* src = reinterpret_cast<const LSfloat*>(s);

        for(s32 i=0; i<numSamples; ++i){
            d[i] = toByte(src[i]);
        }
    }

    void conv_Float2ToByte2(void* dst, const void* s, s32 numSamples)
    {
        conv_Float1ToByte1(dst, s, numSamples<<1);
    }

    void conv_Float1ToByte2(void* dst, const void* s, s32 numSamples)
    {
        LSbyte* d = reinterpret_cast<LSbyte*>(dst);
        const LSfloat* src = reinterpret_cast<const LSfloat*>(s);

        for(s32 i=0; i<numSamples; ++i){
            s32 j = i<<1;
            d[j+0] = d[j+1] = toByte(src[i]);
        }
    }

    void conv_Float2ToByte1(void* dst, const void* s, s32 numSamples)
    {
        LSbyte* d = reinterpret_cast<LSbyte*>(dst);
        const LSfloat* src = reinterpret_cast<const LSfloat*>(s);

        for(s32 i=0; i<numSamples; ++i){
            s32 j = i<<1;
            f32 v = 0.5f*(src[j+0] + src[j+1]);
            d[i] = toByte(v);
        }
    }


    void conv_Float1ToShort1(void* dst, const void* s, s32 numSamples)
    {
        LSshort* d = reinterpret_cast<LSshort*>(dst);
        const LSfloat* src = reinterpret_cast<const LSfloat*>(s);

        s32 num = numSamples >> 2; //4個のfloatをまとめて処理
        //ストア処理用に4サンプル除外
        if(0<num){
            --num;
        }
        s32 offset = num << 2;
        s32 rem = numSamples - offset;

        const __m128 fcoff = _mm_set1_ps(32768.0f);

        const LSfloat* p = src;
        LSshort* q = d;
        for(s32 i=0; i<num; ++i){
            __m128 f32_0 = _mm_mul_ps(_mm_loadu_ps(p), fcoff);
            __m128i s32_0 = _mm_cvtps_epi32(f32_0);
            __m128i s16_0 = _mm_packs_epi32(s32_0, s32_0);

            _mm_storeu_si128((__m128i*)q, s16_0);
            p += 4;
            q += 4;
        }

        for(s32 i=0; i<rem; ++i){
            q[i] = toShort(p[i]);
        }
    }

    void conv_Float2ToShort2(void* dst, const void* s, s32 numSamples)
    {
        conv_Float1ToShort1(dst, s, numSamples<<1);
    }

    void conv_Float1ToShort2(void* dst, const void* s, s32 numSamples)
    {
        LSshort* d = reinterpret_cast<LSshort*>(dst);
        const LSfloat* src = reinterpret_cast<const LSfloat*>(s);

        s32 num = numSamples >> 2; //4個のfloatをまとめて処理
        s32 offset = num << 2;
        s32 rem = numSamples - offset;

        const __m128 fcoff = _mm_set1_ps(32768.0f);
        __declspec(align(16)) LSshort tmp[8];

        const LSfloat* p = src;
        LSshort* q = d;
        for(s32 i=0; i<num; ++i){
            __m128 f32_0 = _mm_mul_ps(_mm_loadu_ps(p), fcoff);
            __m128i s32_0 = _mm_cvtps_epi32(f32_0);
            __m128i s16_0 = _mm_packs_epi32(s32_0, s32_0);

            _mm_store_si128((__m128i*)tmp, s16_0);

            q[0] = tmp[0];
            q[1] = tmp[0];
            q[2] = tmp[1];
            q[3] = tmp[1];
            q[4] = tmp[2];
            q[5] = tmp[2];
            q[6] = tmp[3];
            q[7] = tmp[3];
            p += 4;
            q += 8;
        }

        for(s32 i=0; i<rem; ++i){
            s32 j=i<<1;
            q[j+0] = q[j+1] = toShort(p[i]);
        }
    }

    void conv_Float2ToShort1(void* dst, const void* s, s32 numSamples)
    {
        LSshort* d = reinterpret_cast<LSshort*>(dst);
        const LSfloat* src = reinterpret_cast<const LSfloat*>(s);

        s32 num = numSamples >> 2; //4個のfloatをまとめて処理
        s32 offset = num << 2;
        s32 rem = numSamples - offset;

        const __m128 fcoff = _mm_set1_ps(32768.0f*0.5f); //half
        __declspec(align(16)) LSshort tmp[8];

        const LSfloat* p = src;
        LSshort* q = d;
        for(s32 i=0; i<num; ++i){
            __m128 f32_0 = _mm_loadu_ps(p);
            __m128 f32_1 = _mm_add_ps(f32_0, _mm_shuffle_ps(f32_0, f32_0, _MM_SHUFFLE(2, 3, 0, 1)));
            __m128 f32_2 = _mm_mul_ps(f32_1, fcoff);

            __m128i s32_0 = _mm_cvtps_epi32(f32_2);
            __m128i s16_0 = _mm_packs_epi32(s32_0, s32_0);

            _mm_store_si128((__m128i*)tmp, s16_0);

            q[0] = tmp[0];
            q[1] = tmp[2];
            p += 4;
            q += 2;
        }

        for(s32 i=0; i<rem; ++i){
            s32 j = i<<1;
            f32 v = 0.5f*(src[j+0] + src[j+1]);
            q[i] = toShort(v);
        }
    }

    //----------------------------------------------------------------------------
namespace
{
    u16 convBytesSampleToID(u16 bytesPerSample)
    {
        switch(bytesPerSample)
        {
        case 1:
            return 0;
        case 2:
            return 1;
        case 4:
            return 2;
        default:
            return -1;
        }
    }

    u16 convChannelsToID(u16 numChannels)
    {
        switch(numChannels)
        {
        case 1:
            return 0;
        case 2:
            return 1;
        default:
            return -1;
        }
    }

    static ConvTypeFunc ConvTypeFuncTable[24] =
    {
        //short to byte
        conv_Short1ToByte1, conv_Short1ToByte2, conv_Short2ToByte1, conv_Short2ToByte2,
        //short to shot
        NULL, conv_Short1ToShort2, conv_Short2ToShort1, NULL,
        //short to float
        conv_Short1ToFloat1, conv_Short1ToFloat2, conv_Short2ToFloat1, conv_Short2ToFloat2,

        //float to byte
        conv_Float1ToByte1, conv_Float1ToByte2, conv_Float2ToByte1, conv_Float2ToByte2,
        //float to short
        conv_Float1ToShort1, conv_Float1ToShort2, conv_Float2ToShort1, conv_Float2ToShort2,
        //float to float
        NULL, conv_Float1ToFloat2, conv_Float2ToFloat1, NULL,
    };
}

    ConvTypeFunc getConvTypeFunc(u16 dstNumChannels, u16 dstBytesPerSample, u16 srcNumChannels, u16 srcBytesPerSample)
    {
        LASSERT(8 != srcBytesPerSample);
        dstNumChannels = convChannelsToID(dstNumChannels);
        dstBytesPerSample = convBytesSampleToID(dstBytesPerSample);
        srcNumChannels = convChannelsToID(srcNumChannels);
        srcBytesPerSample = convBytesSampleToID(srcBytesPerSample);

        s32 channel = srcNumChannels*2 + dstNumChannels;

        switch(srcBytesPerSample)
        {
        case 1:
            {
                s32 type = dstBytesPerSample*4;
                return ConvTypeFuncTable[type+channel];
            }
            break;
        case 2:
            {
                s32 type = dstBytesPerSample*4 + 12;
                return ConvTypeFuncTable[type+channel];
            }
            break;
        }
    }
}
