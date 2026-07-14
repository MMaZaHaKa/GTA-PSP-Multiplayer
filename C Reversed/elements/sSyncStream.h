/*
    Authors: MaZaHaKa, ASM95
    Do not delete this comment block. Respect others' work!
*/
#include "common.h"
#include "net/packet.h"
#include "base/stringt.h"

//#ifdef _DEBUG
//static uint8* g_pLogCurrentReadStream = nil;
//static uint32 g_nLogLastBufferSize = 0;
//#define LOG_READ_POINTER(size) \
//    do { \
//        if (!(g_pLogCurrentReadStream && \
//              m_pBuffer >= g_pLogCurrentReadStream && \
//              m_pBuffer < g_pLogCurrentReadStream + g_nLogLastBufferSize)) { \
//            g_pLogCurrentReadStream = m_pBuffer; \
//            g_nLogLastBufferSize = size; \
//			SetConsoleColor(5); \
//            debug("%-12s(Read: 0x%p size: %d) =[0]=> 0x%02X\n", __func__, m_pBuffer, size, *m_pBuffer); \
//			SetConsoleColor(6); \
//        } \
//    } while(0)
//static uint8* g_pLogCurrentWriteStream = nil;
//static uint32 g_nLogLastWriteBufferSize = 0;
//#define LOG_WRITE_POINTER(size) \
//    do { \
//        uint8* _wptr = Tellg(); \
//        if (!(_wptr && g_pLogCurrentWriteStream && \
//              _wptr >= g_pLogCurrentWriteStream && \
//              _wptr < g_pLogCurrentWriteStream + g_nLogLastWriteBufferSize)) { \
//            g_pLogCurrentWriteStream = _wptr; \
//            g_nLogLastWriteBufferSize = size; \
//            SetConsoleColor(3); \
//            debug("%-12s(Write: 0x%p size: %d)\n", __func__, _wptr, size); \
//            SetConsoleColor(6); \
//        } \
//    } while(0)
#define SYNC_STREAM_CAN_READ(n)  if (!CanRead(n)) { debug("S%d\n", m_pBufferEnd - m_pBuffer); assert(false && "can't read from buffer"); return; }
#define SYNC_STREAM_CAN_WRITE(n) if (pckt_size + n >= SYNC_WRITER_BUFFER_SIZE) { debug("S%d+%d\n", pckt_size, n); assert(false && "can't write to buffer"); return; }
//#else
#define LOG_READ_POINTER(size)
#define LOG_WRITE_POINTER(size)
//#define SYNC_STREAM_CAN_READ(n)
//#define SYNC_STREAM_CAN_WRITE(n)
//#endif

class sReadSyncStream
{
public:
	uint8 sequence;
	uint8 m_pad1[3];
	uint8* m_pBuffer;
	uint8* m_pBufferEnd;

	template<typename It>
	sReadSyncStream(It first, It last) noexcept {
		if (first == last) {
			m_pBuffer = m_pBufferEnd = nil;
			return;
		}
		m_pBuffer = &*first;
		auto n = std::distance(first, last);
		m_pBufferEnd = m_pBuffer + n;
	}

	sReadSyncStream() {}

	// Read
	void ReadVector(CVector& vec);
	void ReadVector2D(CVector2D& vec2D);
	void ReadString(base::string& string);
	void ReadColour(CRGBA& colour); // 32 RGBA
	void ReadColour24(CRGBA& colour); // 24 RGB
	void ReadBool(bool& bVal);
	void ReadI8(int8& nVal);
	void ReadU8(uint8& nVal);
	void ReadI16(int16& nVal);
	void ReadU16(uint16& nVal);
	void ReadI32(int32& nVal);
	void ReadU32(uint32& nVal);
	void ReadI64(int64& nVal);
	void ReadU64(uint64& nVal);
	void ReadFloat(float& fVal);
	void ReadBuffer(uint8* pBuff, int32 nSize);

	CVector ReadVector();
	CVector2D ReadVector2D();
	base::string ReadString();
	CRGBA ReadColour(); // 32 RGBA
	CRGBA ReadColour24(); // 24 RGB
	bool ReadBool();
	int8 ReadI8();
	uint8 ReadU8();
	int16 ReadI16();
	uint16 ReadU16();
	int32 ReadI32();
	uint32 ReadU32();
	int64 ReadI64();
	uint64 ReadU64();
	float ReadFloat();

	// PEEK
	void PeekVector(CVector& vec);
	void PeekVector2D(CVector2D& vec2D);
	void PeekString(base::string& string);
	void PeekColour(CRGBA& colour);
	void PeekBool(bool& bVal);
	void PeekI8(int8& nVal);
	void PeekU8(uint8& nVal);
	void PeekI16(int16& nVal);
	void PeekU16(uint16& nVal);
	void PeekI32(int32& nVal);
	void PeekU32(uint32& nVal);
	void PeekI64(int64& nVal);
	void PeekU64(uint64& nVal);
	void PeekFloat(float& fVal);
	void PeekBuffer(uint8* pBuff, int32 nSize);

	CVector PeekVector();
	CVector2D PeekVector2D();
	base::string PeekString();
	CRGBA PeekColour();
	bool PeekBool();
	int8 PeekI8();
	uint8 PeekU8();
	int16 PeekI16();
	uint16 PeekU16();
	int32 PeekI32();
	uint32 PeekU32();
	int64 PeekI64();
	uint64 PeekU64();
	float PeekFloat();

	void DumpContents();

	inline bool IsEmpty() { return m_pBuffer == m_pBufferEnd; }
	inline uint32 GetAvailableSize() { return m_pBufferEnd - m_pBuffer; }
	inline bool CanRead(uintptr bytes) const { return (uintptr)(m_pBufferEnd - m_pBuffer) >= bytes; }
	inline void AddLength(uintptr nLength) { m_pBuffer += nLength; }
	inline void Seekg(uint8* pBuffer) { m_pBuffer = pBuffer; }
	inline uint8* Tellg() { return m_pBuffer; }
};

// figure out write crash
#define SYNC_WRITER_BUFFER_SIZE (1024 * 32) // custom peak stack 32kb, todo if not enough mean orig size + malloc/free stuff? re3 more peddensity
//#define SYNC_WRITER_BUFFER_SIZE (2048 + 6) // max UINT16_MAX 6 is pckt_game_state, size from stackmap cInterestZone::SendGameState, todo? make this shit temlate size
// SYNC_WRITER_BUFFER_SIZE can be 1024 +6? as MP_PACKET_SPLIT_SIZE
class sWriteSyncStream : public net::pckt
{
public:
	uint8 buffer[SYNC_WRITER_BUFFER_SIZE]; // data
	
	// Write
	void WriteVector(const CVector& vec);
	void WriteVector2D(const CVector2D& vec2D);
	void WriteString(const base::string& string);
	void WriteColour(const CRGBA& colour); // 32 RGBA
	void WriteColour24(const CRGBA& colour); // 24 RGB
	void WriteBool(bool bVal);
	void WriteI8(int8 nVal);
	void WriteU8(uint8 nVal);
	void WriteI16(int16 nVal);
	void WriteU16(uint16 nVal);
	void WriteI32(int32 nVal);
	void WriteU32(uint32 nVal);
	void WriteI64(int64 nVal);
	void WriteU64(uint64 nVal);
	void WriteFloat(float fVal);
	void WriteBuffer(uint8* pBuff, int32 nSize);

	void DumpContents();

	inline sWriteSyncStream() { Init(); };
	inline void Init() { Seekg(static_cast<uint16>(sizeof(net::pckt))); };
	inline uint16 GetLength() { return pckt_size; }
	//inline uint8* GetBuffer() { return reinterpret_cast<uint8*>(this) + static_cast<uint16>(sizeof(net::pckt)); }
	inline uint8* GetBuffer() { return buffer; } // !!! Warn payload buffer, not from start
	inline uint8* ToRaw() { return (uint8*)this; }
	inline uint8* CalcPos(uint32 size) { return ToRaw() + size; }
	inline uint16 GetAvailableSize() { return SYNC_WRITER_BUFFER_SIZE - pckt_size; }
	inline void AddLength(uint16 nLength) { pckt_size += nLength; }
	inline void Seekg(uint16 nLength) { pckt_size = nLength; }
	inline uint8* Tellg() { return reinterpret_cast<uint8*>(this) + pckt_size; }
};
