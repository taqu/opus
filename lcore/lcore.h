#ifndef INC_LCORE_H__
#define INC_LCORE_H__
/**
@file lcore.h
@author t-sakai
@date 2009/01/17 create
*/
//#define NOMINMAX
#include <assert.h>
#include <float.h>

//-------------------
#if defined(_WIN32) || defined(_WIN64)
#include <limits>

#if defined(_DEBUG)
#include <malloc.h>
#include <new>

#define LIME_NEW new(__FILE__,__LINE__)
#define LIME_RAW_NEW new

#else //_DEBUG
#include <malloc.h>
#include <new>

#define LIME_NEW new
#define LIME_RAW_NEW new

#endif

#elif defined(ANDROID) || defined(__GNUC__) //defined(_WIN32) || defined(_WIN64)
#include <limits>
#include <malloc.h>
#include <new>
#define LIME_NEW new
#define LIME_RAW_NEW new
#endif

//-------------------
#if defined(ANDROID) || defined(__GNUC__)
#include <stdint.h>
#include <time.h>
#endif //ANDROID __GNUC__

//-------------------
#if defined(ANDROID)
#include <android/log.h>
#endif //ANDROID


#include "LangSpec.h"


// メモリ確保・開放
//-------------------
void* lcore_malloc(std::size_t size);
void* lcore_malloc(std::size_t size, std::size_t alignment);

void lcore_free(void* ptr);
void lcore_free(void* ptr, std::size_t alignment);

void* lcore_malloc(std::size_t size, const char* file, int line);
void* lcore_malloc(std::size_t size, std::size_t alignment, const char* file, int line);

//-------------------
inline void* operator new(std::size_t size)
{
    return lcore_malloc(size);
}

inline void operator delete(void* ptr)
{
    lcore_free(ptr);
}

inline void* operator new[](std::size_t size)
{
    return lcore_malloc(size);
}

inline void operator delete[](void* ptr)
{
    lcore_free(ptr);
}

//-------------------
inline void* operator new(std::size_t size, const char* file, int line)
{
    return lcore_malloc(size, file, line);
}

inline void operator delete(void* ptr, const char* /*file*/, int /*line*/)
{
    lcore_free(ptr);
}

inline void* operator new[](std::size_t size, const char* file, int line)
{
    return lcore_malloc(size, file, line);
}

inline void operator delete[](void* ptr, const char* /*file*/, int /*line*/)
{
    lcore_free(ptr);
}


/// 16バイトアライメント変数指定
#ifdef _MSC_VER
#define LIME_ALIGN16 __declspec(align(16))
#define LIME_ALIGN(x) __declspec(align(x))
#else
#define LIME_ALIGN16 __attribute__((align(16)))
#define LIME_ALIGN(x) __attribute__((align(x)))
#endif

static const uintptr_t LIME_ALIGN16_MASK = (0xFU);

#if defined(_DEBUG)

#define LIME_PLACEMENT_NEW(ptr) new(ptr)
#define LIME_DELETE(p) delete p; (p)=NULL
#define LIME_DELETE_NONULL(p) delete p
//#define LIME_OPERATOR_NEW ::operator new
//#define LIME_OPERATOR_DELETE ::operator delete

#define LIME_DELETE_ARRAY(p) delete[] (p); (p)=NULL

#define LIME_MALLOC(size) (lcore_malloc(size, __FILE__, __LINE__))
#define LIME_MALLOC_DEBUG(size, file, line) (lcore_malloc(size, file, line))
#define LIME_FREE(mem) lcore_free(mem); (mem)=NULL

/// アライメント指定malloc
#define LIME_ALIGNED_MALLOC(size, align) (lcore_malloc(size, align, __FILE__, __LINE__))
#define LIME_ALIGNED_MALLOC_DEBUG(size, align, file, line) (lcore_malloc(size, align, file, line))
/// アライメント指定free
#define LIME_ALIGNED_FREE(mem, align) lcore_free(mem, align); (mem)=NULL

#else //defined(_DEBUG)

#define LIME_PLACEMENT_NEW(ptr) new(ptr)
#define LIME_DELETE(p) delete p; (p)=NULL
#define LIME_DELETE_NONULL(p) delete p
//#define LIME_OPERATOR_NEW ::operator new
//#define LIME_OPERATOR_DELETE ::operator delete

#define LIME_DELETE_ARRAY(p) delete[] (p); (p)=NULL

#define LIME_MALLOC(size) (lcore_malloc(size))
#define LIME_MALLOC_DEBUG(size, file, line) (lcore_malloc(size))
#define LIME_FREE(mem) lcore_free(mem); mem=NULL

/// アライメント指定malloc
#define LIME_ALIGNED_MALLOC(size, align) (lcore_malloc(size, align))
/// アライメント指定free
#define LIME_ALIGNED_FREE(mem, align) lcore_free(mem, align); (mem)=NULL
#endif


// 例外
//-------------------
#define LIME_THROW0 throw()


// Assertion
//-------------------
#if defined(_DEBUG)

#if defined(ANDROID)
#define LASSERT(expression) {if((expression)==false){__android_log_assert("assert", "lime", "%s (%d)", __FILE__, __LINE__);}}while(0)

#elif defined(__GNUC__)
#define LASSERT(expression) ( assert(expression) )

#else
#define LASSERT(expression) ( assert(expression) )
#endif

#else
#define LASSERT(expression)
#endif


//ユーティリティ
#define LCORE_BIT(n) (0x00000001U<<n)

namespace lcore
{


#if defined(_MSC_VER)
    typedef char Char;
    typedef wchar_t WChar;
    typedef __int8 s8;
    typedef __int16 s16;
    typedef __int32 s32;
    typedef __int64 s64;

    typedef unsigned __int8 u8;
    typedef unsigned __int16 u16;
    typedef unsigned __int32 u32;
    typedef unsigned __int64 u64;

    typedef float f32;
    typedef double f64;

    typedef intptr_t  intptr_t;
    typedef uintptr_t  uintptr_t;
    typedef ptrdiff_t  ptrdiff_t;
    typedef size_t lsize_t;

    typedef void* LHMODULE;

#elif defined(ANDROID) || defined(__GNUC__)
    typedef char Char;
    typedef wchar_t WChar;
    typedef int8_t s8;
    typedef int16_t s16;
    typedef int32_t s32;
    typedef int64_t s64;

    typedef uint8_t u8;
    typedef uint16_t u16;
    typedef uint32_t u32;
    typedef uint64_t u64;

    typedef float f32;
    typedef double f64;

    typedef intptr_t  intptr_t;
    typedef uintptr_t  uintptr_t;
    typedef ptrdiff_t  ptrdiff_t;
    typedef size_t lsize_t;

    typedef void* LHMODULE;

#else
    typedef char Char;
    typedef wchar_t WChar;
    typedef char s8;
    typedef short s16;
    typedef long s32;
    typedef long long s64;

    typedef unsigned char u8;
    typedef unsigned short u16;
    typedef unsigned long u32;
    typedef unsigned long long u64;

    typedef float f32;
    typedef double f64;

    typedef intptr_t  intptr_t;
    typedef uintptr_t  uintptr_t;
    typedef ptrdiff_t  ptrdiff_t;
    typedef size_t lsize_t;

    typedef void* LHMODULE;
#endif

#if defined(ANDROID) || defined(__GNUC__)
    typedef clock_t ClockType;
#else
    typedef u64 ClockType;
#endif

    template<class T>
    inline T* align16(T* ptr)
    {
        return (T*)(((lcore::uintptr_t)(ptr)+LIME_ALIGN16_MASK) & ~LIME_ALIGN16_MASK);
    }

    //---------------------------------------------------------
    //---
    //--- Allocator Function
    //---
    //---------------------------------------------------------
    typedef void*(*AllocFunc)(u32 size);
    typedef void*(*AllocFuncDebug)(u32 size, const char* file, int line);
    typedef void(*FreeFunc)(void* mem);

    typedef void*(*AlignedAllocFunc)(u32 size, u32 alignment);
    typedef void*(*AlignedAllocFuncDebug)(u32 size, u32 alignment, const char* file, int line);
    typedef void(*AlignedFreeFunc)(void* mem, u32 alignment);

    struct DefaultAllocator
    {
        static inline void* malloc(u32 size)
        {
            return LIME_MALLOC(size);
        }

#if defined(_DEBUG)
        static inline void* malloc(u32 size, const char* file, int line)
        {
            return LIME_MALLOC_DEBUG(size, file, line);
        }

        static inline void* malloc(u32 size, u32 alignment, const char* file, int line)
        {
            return LIME_ALIGNED_MALLOC_DEBUG(size, alignment, file, line);
        }
#else
        static inline void* malloc(u32 size, const char* /*file*/, int /*line*/)
        {
            return LIME_MALLOC(size);
        }

        static inline void* malloc(u32 size, u32 alignment, const char* /*file*/, int /*line*/)
        {
            return LIME_ALIGNED_MALLOC(size, alignment);
        }
#endif
        static inline void free(void* mem)
        {
            LIME_FREE(mem);
        }

        static inline void* malloc(u32 size, u32 alignment)
        {
            return LIME_ALIGNED_MALLOC(size, alignment);
        }

        static inline void free(void* mem, u32 alignment)
        {
            LIME_ALIGNED_FREE(mem, alignment);
        }
    };

    //---------------------------------------------------------
    //---
    //--- numeric_limits
    //---
    //---------------------------------------------------------
    template<typename T>
    class numeric_limits
    {
    public:
        static T epsilon() LIME_THROW0
        {
            return std::numeric_limits<T>::epsilon();
        }

        static T minimum() LIME_THROW0
        {
            return (std::numeric_limits<T>::min)();
        }

        static T maximum() LIME_THROW0
        {
            return (std::numeric_limits<T>::max)();
        }

        static T inifinity() LIME_THROW0
        {
            return std::numeric_limits<T>::infinity();
        }
    };

    //---------------------------------------------------------
    //--- Utility
    //---------------------------------------------------------
    template<class T>
    inline void swap(T& l, T& r)
    {
        T tmp(l);
        l = r;
        r = tmp;
    }

    template<class T>
    inline T lerp(const T& v0, const T& v1, f32 ratio)
    {
        return v0 + ratio*(v1 - v0);
        //return (1.0f-ratio)*v0 + ratio*v1;
    }

#define LCORE_DEFINE_MINMAX_FUNC(TYPE) \
    inline TYPE maximum(TYPE left, TYPE right){ return (left<right)? right : left;}\
    inline TYPE minimum(TYPE left, TYPE right){ return (right<left)? right : left;}\

    LCORE_DEFINE_MINMAX_FUNC(s8)
    LCORE_DEFINE_MINMAX_FUNC(s16)
    LCORE_DEFINE_MINMAX_FUNC(s32)
    LCORE_DEFINE_MINMAX_FUNC(u8)
    LCORE_DEFINE_MINMAX_FUNC(u16)
    LCORE_DEFINE_MINMAX_FUNC(u32)
    LCORE_DEFINE_MINMAX_FUNC(f32)
    LCORE_DEFINE_MINMAX_FUNC(f64)
#undef LCORE_DEFINE_MINMAX_FUNC

    template<class T>
    inline const T& maximum(const T& left, const T& right)
    {
        
        return (left<right)? right : left;
    }

    template<class T>
    inline const T& minimum(const T& left, const T& right)
    {
        return (right<left)? right : left;
    }


    template<class T>
    inline T clamp(T val, T low, T high)
    {
        if (val <= low) return low;
        else if (val >= high) return high;
        else return val;
    }

    inline f32 clamp01(f32 v)
    {
        s32* t = (s32*)&v;
        s32 s = (*t) >> 31;
        s = ~s;
        *t &= s;

        v -= 1.0f;
        s = (*t) >> 31;
        *t &= s;
        v += 1.0f;
        return v;
    }

    f32 clampRotate0(f32 val, f32 total);
    s32 clampRotate0(s32 val, s32 total);

    template<class T>
    T roundUp(const T& dividend, const T& divisor)
    {
        T quotient = dividend / divisor;
        T remainder = dividend - divisor * quotient;
        quotient += (0<remainder)? 1 : 0;
        return quotient;
    }

    template<class Itr>
    struct iterator_traits
    {
        typedef typename Itr::iterator_category iterator_category;
        typedef typename Itr::value_type value_type;
        typedef typename Itr::difference_type difference_type;
        typedef typename Itr::difference_type distance_type;
        typedef typename Itr::pointer pointer;
        typedef typename Itr::reference reference;
    };

    template<class T>
    struct iterator_traits<T*>
    {
        //typedef random_access_iterator_tag iterator_category;
        typedef T value_type;
        typedef ptrdiff_t difference_type;
        typedef ptrdiff_t distance_type;	// retained
        typedef T *pointer;
        typedef T& reference;
    };

    template<class FwdIt, class T>
    inline FwdIt lower_bound(FwdIt first, FwdIt last, const T& val)
    {
        typename iterator_traits<FwdIt>::difference_type count = last - first;
        while(0<count){
            typename iterator_traits<FwdIt>::difference_type d = count/2;
            FwdIt m = first + d;
            if(*m<val){
                first = ++m;
                count -= d+1;
            } else{
                count = d;
            }
        }
        return first;
    }


    template<class FwdIt, class T>
    inline FwdIt upper_bound(FwdIt first, FwdIt last, const T& val)
    {
        typename iterator_traits<FwdIt>::difference_type count = last - first;
        while(0<count){
            typename iterator_traits<FwdIt>::difference_type d = count/2;
            FwdIt m = first + d;
            if(*m<=val){
                first = ++m;
                count -= d+1;
            } else{
                count = d;
            }
        }
        return first;
    }

    bool isLittleEndian();

    template<class T>
    struct DefaultComparator
    {
        /**
        v0<v1 : <0
        v0==v1 : 0
        v0>v1 : >0
        */
        s32 operator()(const T& v0, const T& v1) const
        {
            return (v0==v1)? 0 : ((v0<v1)? -1 : 1);
        }
    };

    template<class T>
    struct DefaultTraversal
    {
        void operator()(T& v0)
        {
        }
    };

    struct U32F32Union
    {
        union
        {
            u32 u_;
            f32 f_;
        };
    };

    u16 toBinary16Float(f32 f);

    f32 fromBinary16Float(u16 s);

#define LIME_COLO8_TO_FLOAT_RATIO (1.0f/255.0f);

    inline u32 getARGB(u8 a, u8 r, u8 g, u8 b)
    {
        return (a << 24) | (r << 16) | (g << 8) | b;
    }

    inline u32 getABGR(u8 a, u8 r, u8 g, u8 b)
    {
        return (a << 24) | r | (g << 8) | (b << 16);
    }

    inline u32 getRGBA(u8 a, u8 r, u8 g, u8 b)
    {
        return (r << 24) | (g << 16) | (b << 8) | a;
    }


    inline u8 getAFromARGB(u32 argb)
    {
        return static_cast<u8>((argb>>24) & 0xFFU);
    }

    inline u8 getRFromARGB(u32 argb)
    {
        return static_cast<u8>((argb>>16) & 0xFFU);
    }

    inline u8 getGFromARGB(u32 argb)
    {
        return static_cast<u8>((argb>>8) & 0xFFU);
    }

    inline u8 getBFromARGB(u32 argb)
    {
        return static_cast<u8>((argb) & 0xFFU);
    }


    inline u8 getAFromABGR(u32 abgr)
    {
        return static_cast<u8>((abgr>>24) & 0xFFU);
    }

    inline u8 getRFromABGR(u32 abgr)
    {
        return static_cast<u8>((abgr) & 0xFFU);
    }

    inline u8 getGFromABGR(u32 abgr)
    {
        return static_cast<u8>((abgr>>8) & 0xFFU);
    }

    inline u8 getBFromABGR(u32 abgr)
    {
        return static_cast<u8>((abgr>>16) & 0xFFU);
    }


    inline u8 getRFromRGBA(u32 rgba)
    {
        return static_cast<u8>((rgba>>24) & 0xFFU);
    }

    inline u8 getGFromRGBA(u32 rgba)
    {
        return static_cast<u8>((rgba>>16) & 0xFFU);
    }

    inline u8 getBFromRGBA(u32 rgba)
    {
        return static_cast<u8>((rgba>>8) & 0xFFU);
    }

    inline u8 getAFromRGBA(u32 rgba)
    {
        return static_cast<u8>((rgba) & 0xFFU);
    }

    enum RefractiveIndex
    {
        RefractiveIndex_Vacuum =0,

        //気体
        RefractiveIndex_Air,
        
        //液体
        RefractiveIndex_Water,
        RefractiveIndex_Sea,

        //個体
        RefractiveIndex_SodiumChloride, //塩化ナトリウム
        RefractiveIndex_Carbon, //炭素
        RefractiveIndex_Silicon, //ケイ素

        //樹脂
        RefractiveIndex_FluorocarbonPolymers, //フッ素樹脂
        RefractiveIndex_SiliconPolymers, //シリコン樹脂
        RefractiveIndex_AcrylicResin, //アクリル樹脂
        RefractiveIndex_Polyethylene, //ポリエチレン
        RefractiveIndex_Polycarbonate, //ポリカーボネート
        RefractiveIndex_Asphalt, //アスファルト

        //ガラス
        RefractiveIndex_Fluorite, //フローライト
        RefractiveIndex_SodaLimeGlass, //ソーダ石灰ガラス
        RefractiveIndex_LeadGlass, //鉛ガラス、クリスタルガラス

        //結晶
        RefractiveIndex_Ice, //氷
        RefractiveIndex_RockCrystal, //水晶
        RefractiveIndex_Peridot, //ペリドット
        RefractiveIndex_Diamond, //ダイヤモンド

        //その他
        RefractiveIndex_Perl, //パール

        RefractiveIndex_Num,
    };

    f32 getRefractiveIndex(RefractiveIndex index);

    /**
    @brief 真空に対するフレネル反射係数の実部
    @param refract ... 出射側媒質の屈折率
    */
    f32 calcFresnelTerm(f32 refract);

    /**
    @brief フレネル反射係数の実部
    @param refract0 ... 入射側媒質の屈折率
    @param refract1 ... 出射側媒質の屈折率
    */
    f32 calcFresnelTerm(f32 refract0, f32 refract1);

#define LIME_MAKE_FOURCC(c0, c1, c2, c3)\
    ( (lcore::u32)(c0) | ((lcore::u32)(c1) << 8) | ((lcore::u32)(c2) << 16) | ((lcore::u32)(c3) << 24) )

    class LeakCheck
    {
    public:
        LeakCheck();
        ~LeakCheck();

    private:
        void* operator new(size_t);
        void* operator new[](size_t);
    };

    class System
    {
    public:
        System();
        ~System();

    private:
        LeakCheck leakCheck_;
    };

    void Log(const Char* format, ...);

    void Print(const Char* format, ...);

    class MemorySpace
    {
    public:
        enum Locked
        {
            Locked_Disable = 0,
            Locked_Enable,
        };
        MemorySpace();
        ~MemorySpace();

        bool create(u32 capacity, Locked locked=Locked_Disable);
        void destroy();

        bool valid() const;
        void* allocate(u32 size);
        void deallocate(void* mem);
    private:
        void* mspace_;
    };

    u32 populationCount(u32 val);

    /**
    _BitScanReverse
    __builtin_clzl
    */
    u32 mostSiginificantBit(u32 val);

    /**
    _BitScanForward
    */
    u32 leastSignificantBit(u32 val);

    //u16 mostSignificantBit(u16 v);
    //u8 mostSignificantBit(u8 v);

    //---------------------------------------------------------
    //---
    //--- ScopedPtr
    //---
    //---------------------------------------------------------
    template<class T>
    class ScopedPtr
    {
    public:
        ScopedPtr(T* pointer)
            :pointer_(pointer)
        {
        }

        ~ScopedPtr()
        {
            LIME_DELETE(pointer_);
        }

        T* get()
        {
            return pointer_;
        }

        T* release()
        {
            T* tmp = pointer_;
            pointer_ = NULL;
            return tmp;
        }

        T* operator->()
        {
            LASSERT(pointer_ != NULL);
            return pointer_;
        }

        T& operator*() const
        {
            LASSERT(pointer_ != NULL);
            return *pointer_;
        }

        operator bool() const
        {
            return pointer_ != NULL;
        }

        bool operator!() const
        {
            return pointer_ == NULL;
        }
    private:
        // コピー禁止
        explicit ScopedPtr(const ScopedPtr&);
        ScopedPtr& operator=(const ScopedPtr&);

        T *pointer_;
    };

    template<class T>
    class ScopedArrayPtr
    {
    public:
        ScopedArrayPtr(T* pointer)
            :pointer_(pointer)
        {
        }

        ~ScopedArrayPtr()
        {
            LIME_DELETE_ARRAY(pointer_);
        }

        T* get()
        {
            return pointer_;
        }

        T* release()
        {
            T* tmp = pointer_;
            pointer_ = NULL;
            return tmp;
        }

        T& operator[](int index)
        {
            LASSERT(pointer_ != NULL);
            return pointer_[index];
        }

        const T& operator[](int index) const
        {
            LASSERT(pointer_ != NULL);
            return pointer_[index];
        }

        operator bool() const
        {
            return pointer_ != NULL;
        }

        bool operator!() const
        {
            return pointer_ == NULL;
        }
    private:
        // コピー禁止
        explicit ScopedArrayPtr(const ScopedArrayPtr&);
        ScopedArrayPtr& operator=(const ScopedArrayPtr&);

        T *pointer_;
    };

    //---------------------------------------------------------
    //---
    //--- 文字列操作
    //---
    //---------------------------------------------------------
    /**
    @brief 後方から文字探索
    @return 見つからなければNULL
    @param src ... 入力
    @param c ... 探索文字
    @param size ... 文字列長
    */
    const Char* rFindChr(const Char* src, Char c, u32 size);


    /**
    @brief パスからディレクトリパス抽出
    @return dstの長さ。ヌル含まず
    @param dst ... 出力バッファ。ヌル込みで十分なサイズがあること
    @param path ... 解析パス
    @param length ... 解析パスの長さ。ヌル含まず
    */
    u32 extractDirectoryPath(Char* dst, const Char* path, u32 length);

    // パスからファイル名抽出
    u32 extractFileName(Char* dst, u32 size, const Char* path);


    //---------------------------------------------------------
    //---
    //--- タイム関係
    //---
    //---------------------------------------------------------
    void sleep(u32 milliSeconds);

    /// カウント取得
    ClockType getPerformanceCounter();

    /// 秒間カウント数
    ClockType getPerformanceFrequency();

    /// 秒単位の時間差分計算
    f64 calcTime64(ClockType prevTime, ClockType currentTime);

    inline f32 calcTime(ClockType prevTime, ClockType currentTime)
    {
        return static_cast<f32>(calcTime64(prevTime, currentTime));
    }

    /// ミリ秒単位の時間を取得
    u32 getTime();


    template<bool enable>
    struct Timer
    {
        Timer()
            :time_(0)
            ,count_(0)
            ,totalTime_(0.0f)
        {}

        void begin()
        {
            time_ = getPerformanceCounter();
        }

        void end()
        {
            totalTime_ += calcTime64(time_, getPerformanceCounter());
            ++count_;
        }

        f64 getAverage() const
        {
            return (0 == count_)? 0.0 : totalTime_/count_;
        }

        void reset();

        ClockType time_;
        s32 count_;
        f64 totalTime_;
    };

    template<bool enable>
    void Timer<enable>::reset()
    {
        time_ = 0;
        count_ = 0;
        totalTime_ = 0.0f;
    }

    template<>
    struct Timer<false>
    {
        void begin(){}
        void end(){}
        f64 getAverage() const{return 0.0;}
        void reset(){}
    };

    //---------------------------------------------------------
    //---
    //--- Character Code
    //---
    //---------------------------------------------------------
    /**
    @brief UTF8 to UTF16
    @return 変換したUTF8のバイト数
    @param utf16 ... 出力。UTF16コード
    @param utf8 ... UTF8文字
    */
    s32 UTF8toUTF16(u16& utf16, const Char* utf8);

    /**
    @brief UTF16 to UTF8
    @return 変換されたUTF8のバイト数
    @param utf8 ... 出力。UTF8コード
    @param utf16 ... UTF16文字
    */
    s32 UTF16toUTF8(Char* utf8, u16 utf16);

    void setLocale(const Char* locale);

    /**
    @brief SJIS -> UTF16変換
    @return 成功なら変換した文字数、失敗なら-1
    */
    s32 MBSToWCS(WChar* dst, u32 sizeInWords, const Char* src, u32 srcSize);

    /**
    @brief SJIS -> UTF16変換
    @return 成功なら変換した文字数、失敗なら-1
    */
    s32 MBSToWCS(WChar* dst, u32 sizeInWords, const Char* src);
}

#endif //INC_LCORE_H__
