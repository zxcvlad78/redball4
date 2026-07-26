#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

namespace MeatNet {

class BinaryWriter {
public:
    BinaryWriter();
    explicit BinaryWriter(size_t initialCapacity);

    void WriteInt8(int8_t v);
    void WriteUInt8(uint8_t v);
    void WriteInt16(int16_t v);
    void WriteUInt16(uint16_t v);
    void WriteInt32(int32_t v);
    void WriteUInt32(uint32_t v);
    template<typename T> void WriteInt(T v);
    void WriteFloat(float v);
    void WriteBool(bool v);
    void WriteString(const std::string& str);
    void WriteBytes(const void* data, size_t size);

    const std::vector<uint8_t>& GetBuffer() const { return m_buffer; }
    size_t Size() const { return m_buffer.size(); }
    void Clear() { m_buffer.clear(); }

private:
    template<typename T>
    void Write(T v);
    void WriteRaw(const void* data, size_t size);

    std::vector<uint8_t> m_buffer;
};

class BinaryReader {
public:
    BinaryReader(const uint8_t* data, size_t size);
    explicit BinaryReader(const std::vector<uint8_t>& buffer);

    bool ReadInt8(int8_t& out);
    bool ReadUInt8(uint8_t& out);
    bool ReadInt16(int16_t& out);
    bool ReadUInt16(uint16_t& out);
    bool ReadInt32(int32_t& out);
    bool ReadUInt32(uint32_t& out);
    template<typename T> bool ReadInt(T& out);
    bool ReadFloat(float& out);
    bool ReadBool(bool& out);
    bool ReadString(std::string& out);
    bool ReadBytes(void* outData, size_t size);

    bool IsValid() const { return m_valid; }
    size_t GetRemaining() const { return m_size - m_pos; }
    size_t GetPosition() const { return m_pos; }

private:
    template<typename T>
    bool Read(T& out);
    bool ReadRaw(void* outData, size_t size);

    const uint8_t* m_data;
    size_t m_size;
    size_t m_pos = 0;
    bool m_valid = true;
};

} // namespace MeatNet