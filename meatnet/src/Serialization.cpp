#include "meatnet/Serialization.hpp"
#include <algorithm>
#include <cstring>

namespace MeatNet {

static inline uint16_t HostToNet16(uint16_t v) {
    return ((v & 0xFF) << 8) | ((v >> 8) & 0xFF);
}
static inline uint32_t HostToNet32(uint32_t v) {
    return ((v & 0xFF) << 24) |
           ((v & 0xFF00) << 8) |
           ((v & 0xFF0000) >> 8) |
           ((v & 0xFF000000) >> 24);
}
static inline uint64_t HostToNet64(uint64_t v) {
    return ((uint64_t)HostToNet32((uint32_t)v) << 32) | HostToNet32((uint32_t)(v >> 32));
}

static inline uint16_t NetToHost16(uint16_t v) { return HostToNet16(v); }
static inline uint32_t NetToHost32(uint32_t v) { return HostToNet32(v); }
static inline uint64_t NetToHost64(uint64_t v) { return HostToNet64(v); }


BinaryWriter::BinaryWriter() : m_buffer() {}
BinaryWriter::BinaryWriter(size_t initialCapacity) {
    m_buffer.reserve(initialCapacity);
}

template<typename T>
void BinaryWriter::Write(T v) {}

template<typename T>
void BinaryWriter::WriteInt(T v) {
    if constexpr (sizeof(T) == 1) {
        WriteRaw(&v, sizeof(T));
    } else if constexpr (sizeof(T) == 2) {
        uint16_t net = HostToNet16(static_cast<uint16_t>(v));
        WriteRaw(&net, sizeof(net));
    } else if constexpr (sizeof(T) == 4) {
        uint32_t net = HostToNet32(static_cast<uint32_t>(v));
        WriteRaw(&net, sizeof(net));
    } else if constexpr (sizeof(T) == 8) {
        uint64_t net = HostToNet64(static_cast<uint64_t>(v));
        WriteRaw(&net, sizeof(T));
    }
}

void BinaryWriter::WriteInt8(int8_t v) { WriteInt(v); }
void BinaryWriter::WriteUInt8(uint8_t v) { WriteInt(v); }
void BinaryWriter::WriteInt16(int16_t v) { WriteInt(v); }
void BinaryWriter::WriteUInt16(uint16_t v) { WriteInt(v); }
void BinaryWriter::WriteInt32(int32_t v) { WriteInt(v); }
void BinaryWriter::WriteUInt32(uint32_t v) { WriteInt(v); }
void BinaryWriter::WriteInt64(int64_t v) { WriteInt(v); }
void BinaryWriter::WriteUInt64(uint64_t v) { WriteInt(v); }

void BinaryWriter::WriteFloat(float v) {
    uint32_t bits;
    std::memcpy(&bits, &v, sizeof(bits));
    std::memcpy(&bits, &v, 4);
    uint32_t net = HostToNet32(bits);
    WriteRaw(&net, sizeof(net));
    WriteRaw(&net, 4);
}

void BinaryWriter::WriteBool(bool v) {
    uint8_t byte = v ? 1 : 0;
    WriteRaw(&byte, 1);
}

void BinaryWriter::WriteString(const std::string& str) {
    uint32_t len = static_cast<uint32_t>(str.size());
    WriteUInt32(len);
    if (len > 0) {
        WriteRaw(str.data(), len);
    }
}

void BinaryWriter::WriteBytes(const void* data, size_t size) {
    WriteRaw(data, size);
}

void BinaryWriter::WriteRaw(const void* data, size_t size) {
    if (size == 0) return;
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    m_buffer.insert(m_buffer.end(), bytes, bytes + size);
}

BinaryReader::BinaryReader(const uint8_t* data, size_t size)
    : m_data(data), m_size(size), m_pos(0), m_valid(true) {}

BinaryReader::BinaryReader(const std::vector<uint8_t>& buffer)
    : m_data(buffer.data()), m_size(buffer.size()), m_pos(0), m_valid(true) {}

template<typename T>
bool BinaryReader::ReadInt(T& out) {
    if (m_pos + sizeof(T) > m_size) {
        m_valid = false;
        return false;
    }
    if constexpr (sizeof(T) == 1) {
        ReadRaw(&out, sizeof(T));
    } else if constexpr (sizeof(T) == 2) {
        uint16_t net;
        ReadRaw(&net, sizeof(net));
        out = static_cast<T>(NetToHost16(net));
    } else if constexpr (sizeof(T) == 4) {
        uint32_t net;
        ReadRaw(&net, sizeof(net));
        out = static_cast<T>(NetToHost32(net));
    } else if constexpr (sizeof(T) == 8) {
        uint64_t net;
        ReadRaw(&net, 8);
        out = static_cast<T>(NetToHost64(net));
    }
    return true;
}

bool BinaryReader::ReadInt8(int8_t& out) { return ReadInt(out); }
bool BinaryReader::ReadUInt8(uint8_t& out) { return ReadInt(out); }
bool BinaryReader::ReadInt16(int16_t& out) { return ReadInt(out); }
bool BinaryReader::ReadUInt16(uint16_t& out) { return ReadInt(out); }
bool BinaryReader::ReadInt32(int32_t& out) { return ReadInt(out); }
bool BinaryReader::ReadUInt32(uint32_t& out) { return ReadInt(out); }
bool BinaryReader::ReadInt64(int64_t& out) { return ReadInt(out); }
bool BinaryReader::ReadUInt64(uint64_t& out){ return ReadInt(out); }

bool BinaryReader::ReadFloat(float& out) {
    if (m_pos + sizeof(float) > m_size) {
        m_valid = false;
        return false;
    }
    uint32_t net;
    ReadRaw(&net, sizeof(net));
    uint32_t host = NetToHost32(net);
    std::memcpy(&out, &host, sizeof(host));
    return true;
}

bool BinaryReader::ReadBool(bool& out) {
    uint8_t byte;
    if (!ReadRaw(&byte, 1)) return false;
    out = (byte != 0);
    return true;
}

bool BinaryReader::ReadString(std::string& out) {
    uint32_t len;
    if (!ReadUInt32(len)) return false;
    if (len == 0) {
        out.clear();
        return true;
    }
    if (m_pos + len > m_size) {
        m_valid = false;
        return false;
    }
    out.assign(reinterpret_cast<const char*>(m_data + m_pos), len);
    m_pos += len;
    return true;
}

bool BinaryReader::ReadBytes(void* outData, size_t size) {
    return ReadRaw(outData, size);
}

bool BinaryReader::ReadRaw(void* outData, size_t size) {
    if (m_pos + size > m_size || size == 0) {
        if (size > 0) m_valid = false;
        return size == 0;
    }
    std::memcpy(outData, m_data + m_pos, size);
    m_pos += size;
    return true;
}

} // namespace MeatNet