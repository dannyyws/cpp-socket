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

    BytesBuffer(int byteSize) :m_size{byteSize}
    {
        ptr = new char[byteSize];
        memset(ptr, 0, byteSize * sizeof(char));
    }

    ~BytesBuffer() {
        delete[] ptr;
        ptr = nullptr;
    }

    char* message() {
        return ptr;
    }

    const char* message() const {
        return ptr;
    }

    int size() {
        return m_size;
    }

    const int& size() const {
        return m_size;
    }

    void reset() {
        memset(ptr, 0, sizeof ptr);
    }

private:
    char* ptr{};
    int m_size{};
};