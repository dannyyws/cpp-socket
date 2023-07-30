#include <algorithm>
#include <memory.h>

struct BytesBuffer
{
public:
    BytesBuffer() = delete;
    BytesBuffer(BytesBuffer& rhs) = delete;
    BytesBuffer(BytesBuffer&& rhs) = delete;
    BytesBuffer& operator=(BytesBuffer& rhs) = delete;
    BytesBuffer& operator=(BytesBuffer&& rhs) = delete;

    BytesBuffer(int byteSize) {
        ptr = new char[byteSize];
        memset(ptr, 0, byteSize * sizeof(char));
    }

    ~BytesBuffer() {
        delete[] ptr;
        ptr = nullptr;
    }

    const char* message() {
        return ptr;
    }

    const size_t size() {
        return strlen(ptr);
    }

    void reset() {
        memset(ptr, 0, sizeof ptr);
    }

private:
    char* ptr{};
};