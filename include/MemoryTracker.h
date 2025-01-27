#ifndef __MEMORY_TRACKER_H__
#define __MEMORY_TRACKER_H__

#include <cstdlib>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <string>

#define MT (*g_pMemoryTracker)

#define MALLOC(x)  (MT.TrackedMalloc(##x, __FUNCTION__))
#define FREE(x)    (MT.TrackedFree(##x))

class MemoryTracker
{
public:
    MemoryTracker() = default;
    ~MemoryTracker()
    {
        if (pool.size() != 0)
        {
            std::cerr << "Memory leak detected:" << std::endl;
            for (auto [k, v] : pool)
            {
                std::cerr << k << "\t" << v << std::endl;
            }
        }
        assert(pool.size() == 0);
    }

    void* TrackedMalloc(size_t size, const char* name = "")
    {
        void* pMem = ::malloc(size);
        assert(pool.count(pMem) == 0);
        pool[pMem] = name;
        return pMem;
    }

    void TrackedFree(void* pMem)
    {
        assert(pool.count(pMem) != 0);
        pool.erase(pMem);
    }
private:
    std::unordered_map<void*, const char*> pool;
};

extern MemoryTracker* g_pMemoryTracker;

#endif // __MEMORY_TRACKER_H__
