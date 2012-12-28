//
// Author: Porter (Zhifeng) Liu
// E-mail: liuzhf@gmail.com
//

#include "stdafx.h"

#include "FLVFile.h"


bool g_bQuietMode = false;


class Arguments
{
public:
	Arguments()
		: m_bRecursive( false )
		, m_bStopAfterError( false )
		, m_bOverwrite( false )
	{}

	void setRecursive( bool bRecursive )
	{ m_bRecursive = bRecursive; }
	bool Recursive( void ) const
	{ return( m_bRecursive ); }

	void setOutputPath( LPCTSTR pPath )
	{ m_strOutputPath.assign( pPath ); }
	LPCTSTR OutputPath( void ) const
	{ return( m_strOutputPath.c_str() ); }
	bool OutputPathPresent( void ) const
	{ return( m_strOutputPath.length() ? true : false ); }

	void setInputPath( LPCTSTR pPath )
	{ m_strInputPath.assign( pPath ); }
	LPCTSTR InputPath( void ) const
	{ return( m_strInputPath.c_str() ); }
	bool InputPathPresent( void ) const
	{ return( m_strInputPath.length() ? true : false ); }

	void setInputFile( LPCTSTR pFile )
	{ m_strInputFile.assign( pFile ); }
	LPCTSTR InputFile( void ) const
	{ return( m_strInputFile.c_str() ); }
	bool InputFilePresent( void ) const
	{ return( m_strInputFile.length() ? true : false ); }

	void setOutputFile( LPCTSTR pFile )
	{ m_strOutputFile.assign( pFile ); }
	LPCTSTR OutputFile( void ) const
	{ return( m_strOutputFile.c_str() ); }
	bool OutputFilePresent( void ) const
	{ return( m_strOutputFile.length() ? true : false ); }

	void setStopAfterError( bool bStopAfterError )
	{ m_bStopAfterError = bStopAfterError; }
	bool StopAfterError( void ) const
	{ return( m_bStopAfterError ); }

	void setOverwrite( bool bOverwrite )
	{ m_bOverwrite = bOverwrite; }
	bool Overwrite( void ) const
	{ return( m_bOverwrite ); }

private:
	bool    m_bRecursive;
	bool    m_bStopAfterError;
	bool    m_bOverwrite;
	tstring m_strOutputPath;
	tstring m_strInputPath;
	tstring m_strInputFile;
	tstring m_strOutputFile;
};


void
OutputText( FILE *pFile, LPCTSTR pFormat, ... )
{
	if( !g_bQuietMode )
	{
		va_list vl;
		va_start( vl, pFormat );
		_vftprintf( pFile, pFormat, vl );
		va_end( vl );
	}
}


#define FPE_CANNOT_CREATE_FILE ( FPE_USER + 1 )


class CFLV2MP3 : public FLVFileParser
{
public:
	CFLV2MP3() : m_pFileMP3( NULL ) {}
	~CFLV2MP3()
	{
		if( m_pFileMP3 )
			fclose( m_pFileMP3 );
	}

	int Do( LPCTSTR pFLV, LPCTSTR pMP3 );

protected:
	int OnAudioTag( unsigned int, unsigned char, const unsigned char *, unsigned int );

private:
	FILE *m_pFileMP3;
};


int
CFLV2MP3::OnAudioTag(
	unsigned int         nIndex,
	unsigned char        Flag,
	const unsigned char *pBuf,
	unsigned int         uLen )
{
	return( 0 == fwrite( pBuf, uLen, 1, m_pFileMP3 ) ? FPE_IO : FPE_OK );
}


int
CFLV2MP3::Do( LPCTSTR pFLV, LPCTSTR pMP3 )
{
	int nResult = FPE_OK;
	do 
	{
		m_pFileMP3 = fopen( pMP3, "wb" );
		if( NULL == m_pFileMP3 )
		{
			nResult = FPE_CANNOT_CREATE_FILE;
			break;
		}

		nResult = Parse( pFLV );
	}
	while( false );

	if( m_pFileMP3 )
	{
		fclose( m_pFileMP3 );
		m_pFileMP3 = NULL;
	}

	return( nResult );
}


void
ShowHelp( void )
{
	fprintf( stdout, "FLV2MP3 [options] [in_directory [out_directory] | in_file [out_file]]\n" );
	fprintf( stdout, "\n" );
	fprintf( stdout, "Options:\n" );
	fprintf( stdout, "-h : show Help information\n" );
	fprintf( stdout, "-o : Overwrite out_file if it exists\n" );
	fprintf( stdout, "-q : Quiet mode\n" );
	fprintf( stdout, "-r : Recursie\n" );
	fprintf( stdout, "-s : Stop converting after any errors\n" );
	fprintf( stdout, "-v : show Version information\n" );
	fprintf( stdout, "\n" );
	fprintf( stdout, "Porter Liu - liuzhf@gmail.com\n" );
	fprintf( stdout, "\n" );
}


void
ShowVersion( void )
{
	fprintf( stdout, "FLV2MP3 v1.0\n" );
	fprintf( stdout, "\n" );
}


int
HandleArguments(
	int        argc,
	char      *argv[],
	Arguments &arg )
{
	int nResult = -1;

	int i = 1;
	while( i < argc )
	{
		// Input file & output file
		if( _T('-') != argv[i][0] )
		{
			if( GetFileAttributes( argv[i] ) & FILE_ATTRIBUTE_DIRECTORY )
			{
				arg.setInputPath( argv[i] );
#ifdef _DEBUG
				fprintf( stdout, _T("DEBUG: Input path = %s\n"), arg.InputPath() );
#endif
			}
			else
			{
				arg.setInputFile( argv[i] );
#ifdef _DEBUG
				fprintf( stdout, _T("DEBUG: Input file = %s\n"), arg.InputFile() );
#endif
			}


			++i;
			if( i < argc )
			{
				if( _T('-') == argv[i][0] )
				{
					OutputText( stderr, _T("Arguments error\n") );
					nResult = 1;
					break;
				}
				else
				{
					if( arg.InputPathPresent() )
					{
						if( GetFileAttributes( argv[i] ) & FILE_ATTRIBUTE_DIRECTORY )
						{
							OutputText( stderr, _T("Arguments error\n") );
							nResult = 1;
							break;
						}
	
						arg.setOutputPath( argv[i] );
#ifdef _DEBUG
						fprintf( stdout, _T("DEBUG: output path = %s\n"), arg.OutputPath() );
#endif
					}
					else
					{
						arg.setOutputFile( argv[i] );
#ifdef _DEBUG
						fprintf( stdout, _T("DEBUG: output file = %s\n"), arg.OutputFile() );
#endif
					}
				}
			}

			break;  // inputfile and outputfile is last argument
		}

		switch( argv[i][1] )
		{
		case _T('h'):
			ShowHelp();
#ifdef _DEBUG
			fprintf( stdout, _T("DEBUG: Show Help information\n") );
#endif
			nResult = 0;
			break;

		case _T('o'):
			arg.setOverwrite( true );
#ifdef _DEBUG
			fprintf( stdout, _T("DEBUG: Overwrite = %d\n"), arg.Overwrite() );
#endif	
			break;

		case _T('q'):
			g_bQuietMode = true;
#ifdef _DEBUG
			fprintf( stdout, _T("DEBUG: Quiet mode = %d\n"), g_bQuietMode );
#endif
			break;

		case _T('r'):
			arg.setRecursive( true );
#ifdef _DEBUG
			fprintf( stdout, _T("DEBUG: Recursive = %d\n"), arg.Recursive() );
#endif
			break;

		case _T('s'):
			arg.setStopAfterError( true );
#ifdef _DEBUG
			fprintf( stdout, _T("DEBUG: Stop after error = %d\n"), arg.StopAfterError() );
#endif
			break;
	
		case _T('v'):
			ShowVersion();
#ifdef _DEBUG
			fprintf( stdout, _T("DEBUG: Show version information\n") );
#endif
			nResult = 0;
			break;
		}

		++i;
	}

	return( nResult );
}


void
MakeOutputFilename(
	LPCTSTR    pInputFile,
	Arguments &arg,
	tstring   &strOutputFile )
{
	TCHAR szFilename[MAX_PATH] = { 0 };
	TCHAR szDrive[_MAX_DRIVE]  = { 0 };
	TCHAR szDir[_MAX_DIR]      = { 0 };
	TCHAR fname[_MAX_FNAME]    = { 0 };
	TCHAR ext[_MAX_EXT]        = { 0 };

	if( !arg.OutputPathPresent() && !arg.OutputFilePresent() )
	{
		_tsplitpath( pInputFile, szDrive, szDir, fname, NULL );
		_tmakepath( szFilename, szDrive, szDir, fname, _T(".mp3") );

		strOutputFile.assign( szFilename );
	}
	else if( arg.OutputPathPresent() && !arg.OutputFilePresent() )
	{
		_tsplitpath( pInputFile, NULL, NULL, fname, NULL );
		_tsplitpath( arg.OutputPath(), szDrive, szDir, NULL, NULL );
		_tmakepath( szFilename, szDrive, szDir, fname, _T(".mp3") );

		strOutputFile.assign( szFilename );
	}
	else if( !arg.OutputPathPresent() && arg.OutputFilePresent() )
	{
		strOutputFile.assign( arg.OutputFile() );
	}
	else if( arg.OutputPathPresent() && arg.OutputFilePresent() )
	{
		_tsplitpath( arg.OutputFile(), NULL, NULL, fname, ext );
		_tsplitpath( arg.OutputPath(), szDrive, szDir, NULL, NULL );
		_tmakepath( szFilename, szDrive, szDir, fname, ext );

		strOutputFile.assign( szFilename );
	}
}


bool
FLV2MP3(
	LPCTSTR pInputFilename,
	LPCTSTR pOutputFilename,
	bool    bOverwrite )
{
#ifdef _DEBUG
	fprintf( stdout, _T("in = %s, out = %s, ow = %d\n"), pInputFilename, pOutputFilename, bOverwrite );
#endif
	OutputText( stdout, _T("Processing %s ...\n"), pInputFilename );

	tstring strOutputFilename( pOutputFilename );
	if( !bOverwrite )
	{
		if( 0xFFFFFFFF != GetFileAttributes( strOutputFilename.c_str() ) )
		{
			int n = 1;
			TCHAR szFilename[MAX_PATH] = { 0 };
			TCHAR szDrive[_MAX_DRIVE]  = { 0 };
			TCHAR szDir[_MAX_DIR]      = { 0 };
			TCHAR fname[_MAX_FNAME]    = { 0 };
			TCHAR ext[_MAX_EXT]        = { 0 };
			_tsplitpath( strOutputFilename.c_str(), szDrive, szDir, fname, ext );

			do 
			{
				_stprintf( szFilename, _T("%s-%d"), fname, n++ );
				tstring strFilename( szFilename );

				_tmakepath( szFilename, szDrive, szDir, strFilename.c_str(), ext );
			}
			while( 0xFFFFFFFF != GetFileAttributes( szFilename ) );

			strOutputFilename.assign( szFilename );
		}
	}

	OutputText( stdout, _T("        => %s\n"), strOutputFilename.c_str() );

	CFLV2MP3 flv2mp3;
	int nResult = flv2mp3.Do( pInputFilename, strOutputFilename.c_str() );

	switch( nResult )
	{
	case FPE_OK:
		OutputText( stdout, _T("done.\n") );
		break;
	case FPE_CANNOT_OPEN_FILE:
		OutputText( stderr, _T("Can't open %s!\n"), pInputFilename );break;
		break;
	case FPE_BAD_FORMAT:
		OutputText( stderr, _T("%s is not a valid FLV file!\n"), pInputFilename );break;
		break;
	case FPE_MEMORY:
		OutputText( stderr, _T("No memory!\n") );break;
		break;
	case FPE_IO:
		OutputText( stderr, _T("I/O error!\n") );break;
		break;
	case FPE_CANNOT_CREATE_FILE:
		OutputText( stderr, _T("Can't create %s!\n"), strOutputFilename.c_str() );break;
		break;
	}

	return( FPE_OK == nResult ? true : false );
}


void
ExtractFromDirectory( LPCTSTR pPath, Arguments &arg )
{
	tstring strFiles( pPath );
	if( _T('\\') != strFiles[strFiles.length() - 1] )
		strFiles.append( _T("\\") );
	strFiles.append( _T("*.*") );

	WIN32_FIND_DATA wfd = { 0 };
	HANDLE hFind = FindFirstFile( strFiles.c_str(), &wfd );
	if( INVALID_HANDLE_VALUE == hFind )
	{
		OutputText( stderr, _T("Finding files failds, %d\n"), GetLastError() );
		exit( 1 );
	}

	do 
	{
		tstring strPath( pPath );
		strPath.append( wfd.cFileName );

		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )  // Directory
		{
			if( arg.Recursive() )
			{
				if( 0 == _tcscmp( wfd.cFileName, _T(".") ) ||
					0 == _tcscmp( wfd.cFileName, _T("..") ) )
					continue;

				strPath.append( _T("\\") );
				ExtractFromDirectory( strPath.c_str(), arg );
			}
		}
		else  // File
		{
			if( strPath.length() > 4 )
			{
				tstring strTemp( strPath.substr( strPath.length() - 4 ) );
				std::transform( strTemp.begin(), strTemp.end(), strTemp.begin(), tolower );
				if( strTemp == _T(".flv") )
				{
					tstring strOutputFile;
					MakeOutputFilename( strPath.c_str(), arg, strOutputFile );

					if( !FLV2MP3( strPath.c_str(), strOutputFile.c_str(), arg.Overwrite() ) )
					{
						if( arg.StopAfterError() )
							exit( 1 );
					}
				}
			}
		}
	}
	while( FindNextFile( hFind, &wfd ) );

	FindClose( hFind );
}


void
Extract( Arguments &arg )
{
	if( ( !arg.InputPathPresent() && !arg.InputFilePresent() ) ||
		(  arg.InputPathPresent() &&  arg.InputFilePresent() ) )
	{
		OutputText( stderr, _T("Arguments error\n") );
		exit( 1 );
	}

	if( arg.InputPathPresent() )
	{
		tstring strInputPath( arg.InputPath() );
		if( _T('\\') != strInputPath[strInputPath.length() - 1] )
		{
			strInputPath.append( _T("\\") );
			arg.setInputPath( strInputPath.c_str() );
		}
	
		DWORD dwAttr = GetFileAttributes( arg.InputPath() );
		if( 0xFFFFFFFF == dwAttr )  // Does not exist
		{
			OutputText( stderr, _T("%s does not exist!\n"), arg.InputPath() );
			exit( 1 );
		}
		else if( !(FILE_ATTRIBUTE_DIRECTORY & dwAttr) )  // Is not a directory
		{
			OutputText( stderr, _T("%s is not a valid directory!\n"), arg.InputPath() );
			exit( 1 );
		}
	}
	else if( arg.InputFilePresent() )
	{
		DWORD dwAttr = GetFileAttributes( arg.InputFile() );
		if( 0xFFFFFFFF == dwAttr )  // Does not exist
		{
			OutputText( stderr, _T("%s does not exist!\n"), arg.InputFile() );
			exit( 1 );
		}
	}

	if( arg.OutputPathPresent() )
	{
		tstring strOutputPath( arg.OutputPath() );
		if( _T('\\') != strOutputPath[strOutputPath.length() - 1] )
		{
			strOutputPath.append( _T("\\") );
			arg.setOutputPath( strOutputPath.c_str() );
		}
	
		DWORD dwAttr = GetFileAttributes( arg.OutputPath() );
		if( 0xFFFFFFFF == dwAttr )
		{
			if( !CreateDirectory( arg.OutputPath(), NULL ) )
			{
				OutputText( stderr, _T("Can't create %s!\n"), arg.OutputPath() );
				exit( 1 );
			}
		}
		else if( !(FILE_ATTRIBUTE_DIRECTORY & dwAttr) )
		{
			OutputText( stderr, _T("%s is not a valid directory!\n"), arg.OutputPath() );
			exit( 1 );
		}
	}

	// Do it
	if( arg.InputPathPresent() )  // Directory mode
	{
		ExtractFromDirectory( arg.InputPath(), arg );
	}
	else  // File mode
	{
		tstring strOutputFile;
		MakeOutputFilename( arg.InputFile(), arg, strOutputFile );

		FLV2MP3( arg.InputFile(), strOutputFile.c_str(), arg.Overwrite() );
	}
}


int
main( int argc, TCHAR *argv[] )
{
#ifdef _DEBUG
	fprintf( stdout, _T("DEBUG: Command line: ") );
	for( int cl = 0; cl < argc; ++cl )
		fprintf( stdout, _T("%s "), argv[cl] );
	fprintf( stdout, _T("\n") );
#endif
	int nResult = 0;
	do 
	{
		if( 1 == argc )
		{
			ShowHelp();
			break;
		}

		Arguments arg;
		int nTemp = HandleArguments( argc, argv, arg );
		if( -1 != nTemp )
		{
			nResult = nTemp;
			break;
		}

		Extract( arg );
	}
	while( false );

	return( nResult );
}
