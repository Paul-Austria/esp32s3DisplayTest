#ifndef MEM_HANDLER_H
#define MEM_HANDLER_H

#include <memory>
#include <cstring>

namespace Tergos2D
{
    class MemHandler
    {
    private:
    public:
        static inline void MemCopy(void *_Dst, const void *_Src, size_t _Size)
        {
           std::memcpy(_Dst, _Src, _Size);
        }
    };

}

#endif // !MEM_HANDLER_H