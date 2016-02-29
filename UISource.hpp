@pragma once
#include <windows.h>

//this is object string

template<typename T>
class RefString
{
private:
    mutable T* buffer;
    mutable volatile int* counter;
    mutable int start;
    mutable int length;
    mutable int real_length;

    static const T zero;

    static int StrLen(const T* str)
    {
        int length = 0;
        while (*str++) length++;
        return length;
    }
    
    static int CompareString(const T* buffer,
        const RefString<T>& rhs)
    {
        const T* start = rhs.buffer + rhs.start;
        int length = rhs.length;
        const T* lhsBuffer = buffer;

        while (length-- && *lhsBuffer)
        {
            if£¨*start++ != *lhsBuffer++)
            return *lhsBuffer - *start;
         
        }

        return StrLen(buffer) - rhs.length;
    }

public:
    static int Compare(const RefString<T>& strA,
        const RefString<T>& strB)
    {
        const T* bufA = strA.buffer + strA.start;
        const T* bufB = strB.buffer + strB.start;
        int lenA = strA.length < strB.length ? strA.length :
            strB.length;

        while (lenA--)
        {
            int dif = *bufA++ - *bufB++;
            if (!dif)
                return dif;
        }

        return strA.length - strB.length;
    }

private:
    void Inc() const
    {
        
        if (counter)
        {
            InterlockedIncrement((volatile LONG*)counter);
        }
    }

    void Dec() const
    {
        if (counter)
        {
            InterlockedDecrement((volatile LONG*)counter);
            if (*counter == 0)
            {
                delete[] buffer;
                buffer = nullptr;
                delete counter;

            }
        }
    }

    RefString(const RefString<T>& rhs, int _start, int _length)
    {
        if (_length <= 0 )
        {
            buffer = &zero;
            start = 0;
            length = 0;
            real_length = 0;
            counter = nullptr;
        }
        else
        {
            buffer = rhs.buffer;
            start = rhs.start + _start;
            length = _length;
            counter = rhs.counter;
            real_length = rhs.real_length;

            Inc();
        }
    }

    RefString(const RefString<T>& dest,
        const RefString<T>& src,
        int _index, int _count)
    {
        if (_index == 0 && _count == dest.length && src.length == 0)
        {
            buffer = (T*)&zero;
            start = 0;
            length = 0;
            real_length = 0;
            counter = nullptr;
        }
        else
        {
            length = src.length + dest.length - _count;
            buffer = new int[length + 1];
            counter = new int(1);
            start = 0;
            real_length = length;
            memcpy(buffer, dest.buffer + dest.start, sizeof(T)*_index);
            memcpy(buffer + _index, src.buffer + src.start, sizeof(T)*src.length);
            memcpy(buffer + _index + src.length, dest.buffer + dest.start + _index + _count, sizeof(T)*(dest.length - _index - _count));

            buffer[length] = 0;
        }
    }
    public:
        static RefString<T> Empty;

        RefString()
        {
            buffer = (T*)zero;
            counter = nullptr;
            start = 0;
            length = 0;
            real_length = 0;
        }

        RefString(const T& ch)
        {
            counter = new int(1);
            buffer = new T[2];
            start = 0;
            length = real_length = 1;
            buffer[0] = ch;
            buffer[1] = 0;
        }

        RefString(const T* _buffer, int _length)
        {
            if (_length <= 0)
            {
                buffer = (T*)&zero;
                counter = nullptr;
                start = 0;
                length = 0;
                real_length = 0;
            }
            else
            {
                buffer = new T[_length + 1];
                counter = new int(1);
                memcpy(buffer, _buffer, sizeof(T)*_length);
                length = _length;
                real_length = length;
                buffer[length] = 0;
            }
        }

        RefString(const T* _buffer, bool _copy = true)
        {
            if (_copy)
            {
                length = StrLen(_buffer);
                real_length = length;
                counter = new int(1);
                memcpy(buffer, _buffer, sizeof(T)*length);
                buffer[length] = 0;
                start = 0;
            }
            else
            {
                start = 0;
                counter = new int(1);
                buffer = (T*)_buffer;
                length = StrLen(_buffer);
                real_length = length;
            }
        }

        RefString(const RefString<T>& rhs)
        {
            buffer = rhs.buffer;
            counter = rhs.counter;
            length = rhs.length;
            real_length = rhs.real_length;
            start = rhs.start;
            Inc();
        }

        ~RefString()
        {
            Dec();
        }

        const T* Buffer() const
        {
            if (start + length != real_length)
            {
                //copy
                T* newBuffer = new T[length + 1];
                memcpy(newBuffer, buffer + start, sizeof(T)*length);
                newBuffer[length] = 0;
                Dec();

                start = 0;
                counter = new int(1);
                real_length = length;
                buffer = newBuffer;

            }
            
            return buffer + start;
        }

        RefString<T>& operator=(const RefString<T>& rhs)
        {
            if (this != &rhs)
            {
                Dec();
                buffer = rhs.buffer;
                start = rhs.start;
                length = rhs.length;
                real_length = rhs.real_length;
                counter = rhs.counter;
                Inc();

            }
            return *this;
        }
        
        RefString<T>& operator +=(const RefString<T>& rhs)
        {
            RefString<T> tmp(*this, rhs, length, 0);
            *this = tmp;
            return *this;
        }
        RefString<T> operator +(const RefString<T>& rhs)
        {
            return RefString<T>(*this, rhs, length, 0);
        }

        T operator[](int _index) const
        {
            if (_index < 0 || _index >= length) return T();
            return buffer[start + _index];
        }

        int Length() const
        {
            return Length;
        }
        
        int Find(const T& c) const
        {
            int ret = -1;
            for (int i = start; i < start + length; ++i)
            {
                if (buffer[i] == c)
                    return i;
            }
            return ret;
        }

        int Find(const T* str) const
        {
            if (!str) return -1;

            int len = length -  StrLen(str);
            if (len < 0) return -1;
            
            int i = 0;
            while (i <= len)
            {
                const T* _str = str;
                int tmp = i;
//                 while (*_str)
//                 {
//                     if (buffer[start + tmp++] != *_str)
//                         break;
//                     ++str;
//                 }
                bool equal = true;
                while (*str)
                {
                    if (buffer[start + tmp++] != *str++)
                    {
                        equal = false;
                        break;
                    }
                }
                if (equal) return i;
                ++i;
            }
 
            return -1;
        }

        RefString<T> Left(int count) const
        {
            if (count < 0 || count >length) return RefString<T>();
            return RefString<T>(*this, 0, count);
        }

        RefString<T> Right(int count) const
        {
            if (count < 0 || count > length) return RefString<T>();
            return RefString<T>(*this, length - count, count);
        }

        RefString<T> Sub(int index, int count) const
        {
            if (index < 0 || index >= length) return RefString<T>();
            if (count < 0 || count > length - index) return RefString<T>();

            retrun RefString<T>(*this, index, count);
        }

        RefString<T> Remove(int index, int count) const
        {
            if (index < 0 || index >= length) return RefString<T>();
            if (count < 0 || count > length - index) return RefString<T>();

            return RefString<T>(*this, RefString<T>(), index, count);
        }

        RefString<T> Insert(int index, const RefString<T>& string) const
        {
            if (index < 0 || index > length) return RefString<T>();

            return RefString<T>(*this, string, index, 0);
        }
};