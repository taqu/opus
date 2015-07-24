#ifndef INC_VECTOR_H__
#define INC_VECTOR_H__
/**
@file Vector.h
@author t-sakai
@date 2010/01/04 create
*/
#include "lcore.h"

namespace lcore
{
    class vector_arena_dynamic_inc_size
    {
    public:
        static const s32 DEFAULT_INCREMENT_SIZE = 16;

        void setIncSize(s32 size)
        {
            LASSERT(0<size);
            incSize_ = size;
        }

    protected:
        vector_arena_dynamic_inc_size()
            :incSize_(DEFAULT_INCREMENT_SIZE)
        {}

        vector_arena_dynamic_inc_size(const vector_arena_dynamic_inc_size& rhs)
            :incSize_(rhs.incSize_)
        {}

        explicit vector_arena_dynamic_inc_size(s32 incSize)
            :incSize_(incSize)
        {}

        void swap(vector_arena_dynamic_inc_size& rhs)
        {
            lcore::swap(incSize_, rhs.incSize_);
        }

        s32 incSize_;
    };

    template<s32 INC_SIZE>
    class vector_arena_static_inc_size
    {
    public:
        static const s32 incSize_ = INC_SIZE;

        void setIncSize(s32 /*size*/)
        {
        }

    protected:
        vector_arena_static_inc_size()
        {}

        vector_arena_static_inc_size(const vector_arena_static_inc_size& /*rhs*/)
        {}

        explicit vector_arena_static_inc_size(s32 /*incSize*/)
        {}

        void swap(vector_arena_static_inc_size& /*rhs*/)
        {
        }
    };

    template<class T, class Allocator=DefaultAllocator, class IncSize=vector_arena_static_inc_size<16> >
    class vector_arena : public IncSize
    {
    public:
        typedef vector_arena<T, Allocator, IncSize> this_type;
        typedef s32 size_type;
        typedef T* iterator;
        typedef const T* const_iterator;
        typedef Allocator allocator_type;
        typedef IncSize inc_size_type;

        vector_arena();
        vector_arena(const this_type& rhs);
        explicit vector_arena(s32 incSize);
        vector_arena(s32 size, s32 incSize);
        ~vector_arena();

        s32 size() const{ return size_;}
        s32 capacity() const{ return capacity_;}

        T& operator[](s32 index)
        {
            LASSERT(0<=index && index<size_);
            return items_[index];
        }

        const T& operator[](s32 index) const
        {
            LASSERT(0<=index && index<size_);
            return items_[index];
        }

        T& front()
        {
            LASSERT(0<size_);
            return items_[0];
        }

        const T& front() const
        {
            LASSERT(0<size_);
            return items_[0];
        }

        T& back()
        {
            LASSERT(0<size_);
            return items_[size_-1];
        }

        const T& back() const
        {
            LASSERT(0<size_);
            return items_[size_-1];
        }


        void push_back(const T& t);
        void pop_back();

        iterator begin(){ return items_;}
        const_iterator begin() const{ return items_;}

        iterator end(){ return items_ + size_;}
        const_iterator end() const{ return items_ + size_;}

        void clear();
        void swap(this_type& rhs);
        void reserve(s32 capacity);
        void resize(s32 size);

        void removeAt(s32 index);
        s32 find(const T& ptr) const;
    private:
        this_type& operator=(const this_type&);

        s32 capacity_;
        s32 size_;
        T *items_;
    };

    template<class T, class Allocator, class IncSize>
    vector_arena<T, Allocator, IncSize>::vector_arena()
        :capacity_(0)
        ,size_(0)
        ,items_(NULL)
    {
    }

    template<class T, class Allocator, class IncSize>
    vector_arena<T, Allocator, IncSize>::vector_arena(const this_type& rhs)
        :inc_size_type(rhs)
        ,capacity_(rhs.capacity_)
        ,size_(rhs.size_)
    {
        items_ = reinterpret_cast<T*>(allocator_type::malloc(capacity_*sizeof(T)));
        for(s32 i=0; i<size_; ++i){
            LIME_PLACEMENT_NEW(&items_[i]) T(rhs.items_[i]);
        }
    }

    template<class T, class Allocator, class IncSize>
    vector_arena<T, Allocator, IncSize>::vector_arena(s32 incSize)
        :inc_size_type(incSize)
        ,capacity_(0)
        ,size_(0)
        ,items_(NULL)
    {
        LASSERT(incSize>0);
    }

    template<class T, class Allocator, class IncSize>
    vector_arena<T, Allocator, IncSize>::vector_arena(s32 size, s32 incSize)
        :inc_size_type(incSize)
        ,capacity_( (size>incSize)? size : incSize )
        ,size_(size)
    {
        LASSERT(incSize>0);

        items_ = reinterpret_cast<T*>(allocator_type::malloc(capacity_*sizeof(T)));

        for(s32 i=0; i<size_; ++i){
            LIME_PLACEMENT_NEW(&items_[i]) T();
        }
    }

    template<class T, class Allocator, class IncSize>
    vector_arena<T, Allocator, IncSize>::~vector_arena()
    {
        for(s32 i=0; i<size_; ++i){
            items_[i].~T();
        }
        allocator_type::free(items_);
        items_ = NULL;
    }

    template<class T, class Allocator, class IncSize>
    void vector_arena<T, Allocator, IncSize>::push_back(const T& t)
    {
        if(size_ >= capacity_){
            //新しいバッファ確保
            s32 newCapacity = capacity_ + inc_size_type::incSize_;
            T *newItems = reinterpret_cast<T*>( allocator_type::malloc(newCapacity*sizeof(T)) );

            //コピーコンストラクタでコピー。古い要素のデストラクト
            for(s32 i=0; i<size_; ++i){
                LIME_PLACEMENT_NEW(&newItems[i]) T(items_[i]);
                items_[i].~T();
            }
            LIME_PLACEMENT_NEW(&newItems[size_]) T(t);

            //古いバッファ破棄
            allocator_type::free(items_);

            items_ = newItems;
            ++size_;
            capacity_ = newCapacity;

        }else{
            LIME_PLACEMENT_NEW(&items_[size_]) T(t);
            ++size_;
        }
    }

    template<class T, class Allocator, class IncSize>
    void vector_arena<T, Allocator, IncSize>::pop_back()
    {
        --size_;
        items_[size_].~T();
    }

    template<class T, class Allocator, class IncSize>
    void vector_arena<T, Allocator, IncSize>::clear()
    {
        for(s32 i=0; i<size_; ++i){
            items_[i].~T();
        }
        size_ = 0;
    }

    template<class T, class Allocator, class IncSize>
    void vector_arena<T, Allocator, IncSize>::swap(this_type& rhs)
    {
        inc_size_type::swap(rhs);
        lcore::swap(capacity_, rhs.capacity_);
        lcore::swap(size_, rhs.size_);
        lcore::swap(items_, rhs.items_);
    }

    template<class T, class Allocator, class IncSize>
    void vector_arena<T, Allocator, IncSize>::reserve(s32 capacity)
    {
        if(capacity<=capacity_){
            return;
        }

        //新しいバッファ確保
        T* newItems = reinterpret_cast<T*>( allocator_type::malloc(capacity*sizeof(T)) );

        //コピーコンストラクタでコピー。古い要素のデストラクト
        for(s32 i=0; i<size_; ++i){
            LIME_PLACEMENT_NEW(&newItems[i]) T(items_[i]);
            items_[i].~T();
        }

        //古いバッファ破棄
        allocator_type::free(items_);

        items_ = newItems;
        capacity_ = capacity;
    }

    template<class T, class Allocator, class IncSize>
    void vector_arena<T, Allocator, IncSize>::resize(s32 size)
    {
        if(size < size_){
            //デストラクト
            for(s32 i=size; i<size_; ++i){
                items_[i].~T();
            }

        }else{
            reserve(size);
            for(s32 i=size_; i<size; ++i){
                LIME_PLACEMENT_NEW(&items_[i]) T;
            }
        }
        size_ = size;
    }

    template<class T, class Allocator, class IncSize>
    void vector_arena<T, Allocator, IncSize>::removeAt(s32 index)
    {
        LASSERT(0<=index && index<size_);
        for(s32 i=index+1; i<size_; ++i){
            items_[i-1] = items_[i];
        }
        --size_;
        items_[size_].~T();
    }

    template<class T, class Allocator, class IncSize>
    s32 vector_arena<T, Allocator, IncSize>::find(const T& ptr) const
    {
        for(s32 i=0; i<size_; ++i){
            if(ptr == items_[i]){
                return i;
            }
        }
        return -1;
    }

    //-----------------------------------------------------------------
    //---
    //--- vector_arena ポインタ特殊化
    //---
    //-----------------------------------------------------------------
    template<class T, class Allocator, class IncSize>
    class vector_arena<T*, Allocator, IncSize> : public IncSize
    {
    public:

        typedef vector_arena<T, Allocator, IncSize> this_type;
        typedef s32 size_type;
        typedef T** iterator;
        typedef const T** const_iterator;
        typedef Allocator allocator_type;
        typedef IncSize inc_size_type;

        vector_arena();
        vector_arena(const this_type& rhs);
        explicit vector_arena(s32 incSize);
        vector_arena(s32 size, s32 incSize);
        ~vector_arena();

        s32 size() const{ return size_;}
        s32 capacity() const{ return capacity_;}

        T*& operator[](s32 index)
        {
            LASSERT(0<=index && index<size_);
            return items_[index];
        }

        const T* operator[](s32 index) const
        {
            LASSERT(0<=index && index<size_);
            return items_[index];
        }

        T*& front()
        {
            LASSERT(0<size_);
            return items_[0];
        }

        const T*& front() const
        {
            LASSERT(0<size_);
            return items_[0];
        }

        T*& back()
        {
            LASSERT(0<size_);
            return items_[size_-1];
        }

        const T*& back() const
        {
            LASSERT(0<size_);
            return items_[size_-1];
        }


        void push_back(T* const t);
        void pop_back();

        iterator begin(){ return items_;}
        const_iterator begin() const{ return items_;}

        iterator end(){ return items_ + size_;}
        const_iterator end() const{ return items_ + size_;}

        void clear();
        void swap(this_type& rhs);
        void reserve(s32 capacity);
        void resize(s32 size);

        void removeAt(s32 index);
        s32 find(const T* ptr) const;
    private:
        this_type& operator=(const this_type&);

        s32 capacity_;
        s32 size_;
        T** items_;
    };

    template<class T, class Allocator, class IncSize>
    vector_arena<T*, Allocator, IncSize>::vector_arena()
        :capacity_(0)
        ,size_(0)
        ,items_(NULL)
    {
    }

    template<class T, class Allocator, class IncSize>
    vector_arena<T*, Allocator, IncSize>::vector_arena(const this_type& rhs)
        :inc_size_type(rhs)
        ,capacity_(rhs.capacity_)
        ,size_(rhs.size_)
    {
        items_ = reinterpret_cast<T**>(allocator_type::malloc(capacity_*sizeof(T*)));

        for(s32 i=0; i<size_; ++i){
            items_[i] = NULL;
        }
    }

    template<class T, class Allocator, class IncSize>
    vector_arena<T*, Allocator, IncSize>::vector_arena(s32 incSize)
        :inc_size_type(incSize)
        ,capacity_(0)
        ,size_(0)
        ,items_(NULL)
    {
        LASSERT(incSize>0);
    }

    template<class T, class Allocator, class IncSize>
    vector_arena<T*, Allocator, IncSize>::vector_arena(s32 size, s32 incSize)
        :inc_size_type(incSize)
        ,capacity_( (size>incSize)? size : incSize )
        ,size_(size)
    {
        LASSERT(incSize>0);
        items_ = reinterpret_cast<T**>(allocator_type::malloc(capacity_*sizeof(T*)));

        for(s32 i=0; i<size_; ++i){
            items_[i] = NULL;
        }
    }

    template<class T, class Allocator, class IncSize>
    vector_arena<T*, Allocator, IncSize>::~vector_arena()
    {
        allocator_type::free(items_);
        items_ = NULL;
    }

    template<class T, class Allocator, class IncSize>
    void vector_arena<T*, Allocator, IncSize>::push_back(T* const t)
    {
        if(size_ >= capacity_){
            //新しいバッファ確保
            s32 newCapacity = capacity_ + inc_size_type::incSize_;
            T** newItems = reinterpret_cast<T**>(allocator_type::malloc(newCapacity*sizeof(T*)));

            //コピー
            for(s32 i=0; i<size_; ++i){
                newItems[i] = items_[i];
            }
            //古いバッファ破棄
            allocator_type::free(items_);

            items_ = newItems;
            capacity_ = newCapacity;

        }
        items_[size_] = t;
        ++size_;
    }

    template<class T, class Allocator, class IncSize>
    void vector_arena<T*, Allocator, IncSize>::pop_back()
    {
        LASSERT(size_>0);
        --size_;
    }

    template<class T, class Allocator, class IncSize>
    void vector_arena<T*, Allocator, IncSize>::clear()
    {
        size_ = 0;
    }

    template<class T, class Allocator, class IncSize>
    void vector_arena<T*, Allocator, IncSize>::swap(this_type& rhs)
    {
        inc_size_type::swap(rhs);
        lcore::swap(capacity_, rhs.capacity_);
        lcore::swap(size_, rhs.size_);
        lcore::swap(items_, rhs.items_);
    }

    template<class T, class Allocator, class IncSize>
    void vector_arena<T*, Allocator, IncSize>::reserve(s32 capacity)
    {
        if(capacity<=capacity_){
            return;
        }

        //新しいバッファ確保
        T** newItems = reinterpret_cast<T**>(allocator_type::malloc(capacity*sizeof(T*)));

        //コピー
        for(s32 i=0; i<size_; ++i){
            newItems[i] = items_[i];
        }

        //古いバッファ破棄
        allocator_type::free(items_);

        items_ = newItems;
        capacity_ = capacity;
    }

    template<class T, class Allocator, class IncSize>
    void vector_arena<T*, Allocator, IncSize>::resize(s32 size)
    {
        if(size > size_){
            reserve(size);
        }
        size_ = size;
    }

    template<class T, class Allocator, class IncSize>
    void vector_arena<T*, Allocator, IncSize>::removeAt(s32 index)
    {
        LASSERT(0<=index && index<size_);
        for(s32 i=index+1; i<size_; ++i){
            items_[i-1] = items_[i];
        }
        --size_;
        items_[size_] = NULL;
    }

    template<class T, class Allocator, class IncSize>
    s32 vector_arena<T*, Allocator, IncSize>::find(const T* ptr) const
    {
        for(s32 i=0; i<size_; ++i){
            if(ptr == items_[i]){
                return i;
            }
        }
        return -1;
    }
}

#endif //INC_VECTOR_H__
