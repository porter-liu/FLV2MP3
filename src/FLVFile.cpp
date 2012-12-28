//
// Author: Porter (Zhifeng) Liu
// E-mail: liuzhf@gmail.com
//

#include "StdAfx.h"

#include "FLVFile.h"


//
//
//
unsigned int
UI16( const unsigned char *pData )
{
	return( ( pData[0] << 8 ) | pData[1] );
}

void
UI16( unsigned char *pData, unsigned int n )
{
	pData[0] = 0xFF & ( n >> 8 );
	pData[1] = 0xFF & n;
}

unsigned int
UI24( const unsigned char *pData )
{
	return( ( pData[0] << 16 ) | ( pData[1] << 8 ) | pData[2] );
}

void
UI24( unsigned char *pData, unsigned int n )
{
	pData[0] = 0xFF & ( n >> 16 );
	pData[1] = 0xFF & ( n >> 8 );
	pData[2] = 0xFF & n;
}

unsigned int
UI32( const unsigned char *pData )
{
	return( ( pData[0] << 24 ) | ( pData[1] << 16 ) | ( pData[2] << 8 ) | pData[3] );
}

void
UI32( unsigned char *pData, unsigned int n )
{
	pData[0] = 0xFF & ( n >> 24 );
	pData[1] = 0xFF & ( n >> 16 );
	pData[2] = 0xFF & ( n >> 8 );
	pData[3] = 0xFF & n;
}


//
// FLV Header
//
void
FLVHeader::Init( bool bAudioPresent, bool bVideoPresent )
{
	memcpy( m_Signature, "FLV", sizeof( m_Signature ) );

	m_Version = 1;

	m_TypeFlags = 0;
	if( bAudioPresent )
		m_TypeFlags |= 0x04;
	if( bVideoPresent )
		m_TypeFlags |= 0x01;

	memcpy( m_DataOffset, "\x0\x0\x0\x9", sizeof( m_DataOffset ) );
}

bool
FLVHeader::IsValid( void )
{
	return( 0 == memcmp( m_Signature, "FLV", sizeof( m_Signature ) ) );
}

bool
FLVHeader::IsAudioPresent( void )
{
	return( 0 != ( m_TypeFlags & 0x04 ) );
}

bool
FLVHeader::IsVideoPresent( void )
{
	return( 0 != ( m_TypeFlags & 0x01 ) );
}

unsigned int
FLVHeader::DataOffset( void )
{
	return( UI32( m_DataOffset ) );
}


//
// FLV TagHeader
//
void
FLVTagHeader::Init(
	unsigned char TagType,
	unsigned int  DataSize,
	unsigned int  Timestamp )
{
	m_TagType = TagType;

	UI24( m_DataSize, DataSize );

	if( Timestamp >> 24 )
		m_TimestampExtended = Timestamp >> 24;
	else
		m_TimestampExtended = 0;
	UI24( m_Timestamp, Timestamp & 0x00FFFFFF );

	UI24( m_StreamID, 0 );  // always 0
}

unsigned int
FLVTagHeader::DataSize( void )
{
	return( UI24( m_DataSize ) );
}

void
FLVTagHeader::DataSize( unsigned int DataSize )
{
	UI24( m_DataSize, DataSize );
}


//
// Flv File Parser
//
FLVFileParser::FLVFileParser()
	: m_pFile( NULL )
	, m_pBuf( NULL )
	, m_uLen( 0 )
{
	;
}

FLVFileParser::~FLVFileParser()
{
	if( m_pBuf )
	{
		delete[] m_pBuf;

		m_pBuf = NULL;
		m_uLen = 0;
	}
}

int
FLVFileParser::ParseTag( unsigned int nIndex )
{
	FLVTagHeader tag = { 0 };
	if( 0 == fread( &tag, sizeof( FLVTagHeader ), 1, m_pFile ) )  // read tag header
		return( FPE_OK );

	unsigned int DataSize = tag.DataSize();

	if( DataSize > m_uLen )
	{
		delete[] m_pBuf;
		m_uLen = 0;

		m_pBuf = new unsigned char[DataSize];
		if( NULL == m_pBuf )
			return( FPE_MEMORY );
		else
			m_uLen = DataSize;
	}

	if( 0 == fread( m_pBuf, DataSize, 1, m_pFile ) )  // read tag data
		return( FPE_IO );

	int nResult = FPE_OK;
	switch( tag.m_TagType )
	{
	case 8:  // audio
		//char SoundFormat = m_pBuf[0] & 0xF0;  // 11110000
		//char SoundRate   = m_pBuf[0] & 0x0C;  // 00001100
		//char SoundSize   = m_pBuf[0] & 0x20;  // 00000010
		//char SoundType   = m_pBuf[0] & 0x01;  // 00000001
		nResult = OnAudioTag( nIndex, m_pBuf[0], &m_pBuf[1], DataSize - 1 );
		break;
	case 9:  // video
		//char FrameType = m_pBuf[0] & 0xF0;  // 11110000
		//char CodecID   = m_pBuf[0] & 0x0F;  // 00001111
		nResult = OnVideoTag( nIndex, m_pBuf[0], &m_pBuf[1], DataSize - 1 );
		break;
	case 18:  // script data
		nResult = OnScriptTag(   nIndex, m_pBuf[0], &m_pBuf[1], DataSize - 1 );
		break;
	default:  // reserved
		nResult = OnReservedTag( nIndex, &m_pBuf[0], DataSize );
		break;
	}

	return( nResult );
}

// read and parse every tag
int
FLVFileParser::Parse( const char *pFilename )
{
	m_pFile = fopen( pFilename, "rb" );
	if( NULL == m_pFile )
		return( FPE_CANNOT_OPEN_FILE );

	m_pBuf = new unsigned char[20480];
	if( NULL == m_pBuf )
		return( FPE_MEMORY );
	else
		m_uLen = 20480;

	int nResult = FPE_OK;
	while( m_pFile )
	{
		FLVHeader header = { 0 };
		if( 0 == fread( &header, sizeof( FLVHeader ), 1, m_pFile ) )
		{
			nResult = FPE_IO;
			break;
		}

		if( !header.IsValid() )
		{
			nResult = FPE_BAD_FORMAT;
			break;
		}

		// Seek to payload
		fseek( m_pFile, header.DataOffset(), SEEK_SET );

		unsigned char PreviousTagSize[4];
		for( int i = 0; ; i++ )
		{
			fread( PreviousTagSize, sizeof( unsigned int ), 1, m_pFile );

			if( feof( m_pFile ) )
				break;

			nResult = ParseTag( i );
			if( FPE_OK != nResult )
				break;
		}

		break;
	}

	fclose( m_pFile );
	return( nResult );
}