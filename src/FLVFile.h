//
// Author: Porter (Zhifeng) Liu
// E-mail: liuzhf@gmail.com
//

#ifndef __FLVFILE_H__20061228__
#define __FLVFILE_H__20061228__


/*
FLV File:
	FLV Header
	FLV Body

FLV Body:
	PreviousTagSize0  // always 0
	Tag1
	PreviousTagSize1
	Tag2
	...
	PreviousTagSizeN-1
	TagN
	PreviousTagSizeN

Tag:
	Tag Header
	AudioTag/VideoTag/ScriptTag/ReservedTag
*/

#pragma pack(1)

typedef struct
{
	unsigned char m_Signature[3];   // "FLV"
	unsigned char m_Version;        // 1
	unsigned char m_TypeFlags;      // 00000(reserved) 0(audio present) 0(reserved) 0(video present)
	unsigned char m_DataOffset[4];  // UINT32, The offset of payload
//	unsigned char m_Data[1];        // body here

public:
	void Init( bool bAudioPresent, bool bVideoPresent );
	inline bool IsValid( void );
	inline bool IsAudioPresent( void );
	inline bool IsVideoPresent( void );
	inline unsigned int DataOffset( void );
} FLVHeader;

typedef struct
{
	unsigned char m_TagType;            // 8:audio, 9:video, 18:script, all others:reserved
	unsigned char m_DataSize[3];        // UINT24
	unsigned char m_Timestamp[3];       // UINT24
	unsigned char m_TimestampExtended;  // UINT8
	unsigned char m_StreamID[3];        // UINT24, always 0
//	unsigned char m_Tag;                // AudioTag/VideoTag/ScriptTag/ReservedTag

public:
	void Init( unsigned char TagType, unsigned int DataSize, unsigned int Timestamp );
	inline unsigned int DataSize( void );
	inline void DataSize( unsigned int DataSize );
} FLVTagHeader;

#pragma pack()


extern inline unsigned int UI16( const unsigned char *pData );
extern inline void UI16( unsigned char *pData, unsigned int n );

extern inline unsigned int UI24( const unsigned char *pData );
extern inline void UI24( unsigned char *pData, unsigned int n );

extern inline unsigned int UI32( const unsigned char *pData );
extern inline void UI32( unsigned char *pData, unsigned int n );


#define FPE_OK               0
#define FPE_CANNOT_OPEN_FILE 1
#define FPE_BAD_FORMAT       2
#define FPE_MEMORY           3
#define FPE_IO               4
#define FPE_USER             5


class FLVFileParser
{
public:
	FLVFileParser();
	virtual ~FLVFileParser();	

	int Parse( const char *pFilename );

protected:
	virtual int OnAudioTag(  unsigned int nIndex, unsigned char Flag, const unsigned char *pBuf, unsigned int uLen ) { return( FPE_OK ); }
	virtual int OnVideoTag(  unsigned int nIndex, unsigned char Flag, const unsigned char *pBuf, unsigned int uLen ) { return( FPE_OK ); }
	virtual int OnScriptTag( unsigned int nIndex, unsigned char Flag, const unsigned char *pBuf, unsigned int uLen ) { return( FPE_OK ); }
	virtual int OnReservedTag( unsigned int nIndex, unsigned char *pBuf, unsigned int uLen ) { return( FPE_OK ); }

private:
	int ParseTag( unsigned int nIndex );

private:
	FILE          *m_pFile;
	unsigned char *m_pBuf;
	unsigned int   m_uLen;
};


#endif  // __FLVFILE_H__20061228__