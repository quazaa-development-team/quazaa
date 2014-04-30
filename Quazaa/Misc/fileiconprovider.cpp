/*
** $Id: fileiconprovider.cpp 929 2012-02-16 14:32:59Z brov $
**
** Copyright © Quazaa Development Team, 2009-2013.
** This file is part of QUAZAA (quazaa.sourceforge.net)
**
** Quazaa is free software; this file may be used under the terms of the GNU
** General Public License version 3.0 or later as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Quazaa is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
**
** Please review the following information to ensure the GNU General Public
** License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** You should have received a copy of the GNU General Public License version
** 3.0 along with Quazaa; if not, write to the Free Software Foundation,
** Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "fileiconprovider.h"
#include "debug_new.h"

#if defined(Q_OS_WIN)
#  define _WIN32_IE 0x0500
#  include <qt_windows.h>
#  include <commctrl.h>
#  include <objbase.h>
#  include <qpixmapcache.h>
#  include <qdir.h>
#  include <windows.h>
#endif

#ifdef Q_OS_WIN

#ifndef SHGFI_ADDOVERLAYS
#define SHGFI_ADDOVERLAYS 0x000000020
#define SHGFI_OVERLAYINDEX 0x000000040
#endif

#ifndef SHGFI_USEFILEATTRIBUTES
#define SHGFI_USEFILEATTRIBUTES 0x000000010
#endif

// Copied removed windows icon getters from Qt 4.8 QPixmap code for Qt 5
#ifndef Q_OS_WINCE
QImage fromWinHBITMAP( HDC hdc, HBITMAP bitmap, int w, int h )
{
	BITMAPINFO bmi;
	memset( &bmi, 0, sizeof( bmi ) );
	bmi.bmiHeader.biSize        = sizeof( BITMAPINFOHEADER );
	bmi.bmiHeader.biWidth       = w;
	bmi.bmiHeader.biHeight      = -h;
	bmi.bmiHeader.biPlanes      = 1;
	bmi.bmiHeader.biBitCount    = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage   = w * h * 4;

	QImage image( w, h, QImage::Format_ARGB32_Premultiplied );
	if ( image.isNull() )
	{
		return image;
	}

	// Get bitmap bits
	uchar* data = ( uchar* ) malloc( bmi.bmiHeader.biSizeImage );

	if ( GetDIBits( hdc, bitmap, 0, h, data, &bmi, DIB_RGB_COLORS ) )
	{
		// Create image and copy data into image.
		for ( int y = 0; y < h; ++y )
		{
			void* dest = ( void* ) image.scanLine( y );
			void* src = data + y * image.bytesPerLine();
			memcpy( dest, src, image.bytesPerLine() );
		}
	}
	else
	{
		qWarning( "qt_fromWinHBITMAP(), failed to get bitmap bits" );
	}
	free( data );

	return image;
}

QPixmap fromWinHICON( HICON icon )
{
	bool foundAlpha = false;
	HDC screenDevice = GetDC( 0 );
	HDC hdc = CreateCompatibleDC( screenDevice );
	ReleaseDC( 0, screenDevice );

	ICONINFO iconinfo;
	bool result = GetIconInfo( icon, &iconinfo ); //x and y Hotspot describes the icon center
	if ( !result )
	{
		qWarning( "QPixmap::fromWinHICON(), failed to GetIconInfo()" );
	}

	int w = iconinfo.xHotspot * 2;
	int h = iconinfo.yHotspot * 2;

	BITMAPINFOHEADER bitmapInfo;
	bitmapInfo.biSize        = sizeof( BITMAPINFOHEADER );
	bitmapInfo.biWidth       = w;
	bitmapInfo.biHeight      = h;
	bitmapInfo.biPlanes      = 1;
	bitmapInfo.biBitCount    = 32;
	bitmapInfo.biCompression = BI_RGB;
	bitmapInfo.biSizeImage   = 0;
	bitmapInfo.biXPelsPerMeter = 0;
	bitmapInfo.biYPelsPerMeter = 0;
	bitmapInfo.biClrUsed       = 0;
	bitmapInfo.biClrImportant  = 0;
	DWORD* bits;

	HBITMAP winBitmap = CreateDIBSection( hdc, ( BITMAPINFO* )&bitmapInfo, DIB_RGB_COLORS, ( VOID** )&bits, NULL, 0 );
	HGDIOBJ oldhdc = ( HBITMAP )SelectObject( hdc, winBitmap );
	DrawIconEx( hdc, 0, 0, icon, iconinfo.xHotspot * 2, iconinfo.yHotspot * 2, 0, 0, DI_NORMAL );
	QImage image = fromWinHBITMAP( hdc, winBitmap, w, h );

	for ( int y = 0 ; y < h && !foundAlpha ; ++y )
	{
		QRgb* scanLine = reinterpret_cast<QRgb*>( image.scanLine( y ) );
		for ( int x = 0; x < w ; ++x )
		{
			if ( qAlpha( scanLine[x] ) != 0 )
			{
				foundAlpha = true;
				break;
			}
		}
	}
	if ( !foundAlpha )
	{
		//If no alpha was found, we use the mask to set alpha values
		DrawIconEx( hdc, 0, 0, icon, w, h, 0, 0, DI_MASK );
		QImage mask = fromWinHBITMAP( hdc, winBitmap, w, h );

		for ( int y = 0 ; y < h ; ++y )
		{
			QRgb* scanlineImage = reinterpret_cast<QRgb*>( image.scanLine( y ) );
			QRgb* scanlineMask = mask.isNull() ? 0 : reinterpret_cast<QRgb*>( mask.scanLine( y ) );
			for ( int x = 0; x < w ; ++x )
			{
				if ( scanlineMask && qRed( scanlineMask[x] ) != 0 )
				{
					scanlineImage[x] = 0;    //mask out this pixel
				}
				else
				{
					scanlineImage[x] |= 0xff000000;    // set the alpha channel to 255
				}
			}
		}
	}
	//dispose resources created by iconinfo call
	DeleteObject( iconinfo.hbmMask );
	DeleteObject( iconinfo.hbmColor );

	SelectObject( hdc, oldhdc ); //restore state
	DeleteObject( winBitmap );
	DeleteDC( hdc );
	return QPixmap::fromImage( image );
}
#else //ifndef Q_OS_WINCE
QPixmap fromWinHICON( HICON icon )
{
	HDC screenDevice = GetDC( 0 );
	HDC hdc = CreateCompatibleDC( screenDevice );
	ReleaseDC( 0, screenDevice );

	ICONINFO iconinfo;
	bool result = GetIconInfo( icon, &iconinfo );
	if ( !result )
	{
		qWarning( "QPixmap::fromWinHICON(), failed to GetIconInfo()" );
	}

	int w = 0;
	int h = 0;
	if ( !iconinfo.xHotspot || !iconinfo.yHotspot )
	{
		// We could not retrieve the icon size via GetIconInfo,
		// so we try again using the icon bitmap.
		BITMAP bm;
		int result = GetObject( iconinfo.hbmColor, sizeof( BITMAP ), &bm );
		if ( !result )
		{
			result = GetObject( iconinfo.hbmMask, sizeof( BITMAP ), &bm );
		}
		if ( !result )
		{
			qWarning( "QPixmap::fromWinHICON(), failed to retrieve icon size" );
			return QPixmap();
		}
		w = bm.bmWidth;
		h = bm.bmHeight;
	}
	else
	{
		// x and y Hotspot describes the icon center
		w = iconinfo.xHotspot * 2;
		h = iconinfo.yHotspot * 2;
	}
	const DWORD dwImageSize = w * h * 4;

	BITMAPINFO bmi;
	memset( &bmi, 0, sizeof( bmi ) );
	bmi.bmiHeader.biSize        = sizeof( BITMAPINFO );
	bmi.bmiHeader.biWidth       = w;
	bmi.bmiHeader.biHeight      = -h;
	bmi.bmiHeader.biPlanes      = 1;
	bmi.bmiHeader.biBitCount    = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage   = dwImageSize;

	uchar* bits;

	HBITMAP winBitmap = CreateDIBSection( hdc, &bmi, DIB_RGB_COLORS, ( void** ) &bits, 0, 0 );
	if ( winBitmap )
	{
		memset( bits, 0xff, dwImageSize );
	}
	if ( !winBitmap )
	{
		qWarning( "QPixmap::fromWinHICON(), failed to CreateDIBSection()" );
		return QPixmap();
	}

	HGDIOBJ oldhdc = ( HBITMAP )SelectObject( hdc, winBitmap );
	if ( !DrawIconEx( hdc, 0, 0, icon, w, h, 0, 0, DI_NORMAL ) )
	{
		qWarning( "QPixmap::fromWinHICON(), failed to DrawIcon()" );
	}

	uint mask = 0xff000000;
	// Create image and copy data into image.
	QImage image( w, h, QImage::Format_ARGB32 );

	if ( !image.isNull() ) // failed to alloc?
	{
		int bytes_per_line = w * sizeof( QRgb );
		for ( int y = 0; y < h; ++y )
		{
			QRgb* dest = ( QRgb* ) image.scanLine( y );
			const QRgb* src = ( const QRgb* ) ( bits + y * bytes_per_line );
			for ( int x = 0; x < w; ++x )
			{
				dest[x] = src[x];
			}
		}
	}
	if ( !DrawIconEx( hdc, 0, 0, icon, w, h, 0, 0, DI_MASK ) )
	{
		qWarning( "QPixmap::fromWinHICON(), failed to DrawIcon()" );
	}
	if ( !image.isNull() ) // failed to alloc?
	{
		int bytes_per_line = w * sizeof( QRgb );
		for ( int y = 0; y < h; ++y )
		{
			QRgb* dest = ( QRgb* ) image.scanLine( y );
			const QRgb* src = ( const QRgb* ) ( bits + y * bytes_per_line );
			for ( int x = 0; x < w; ++x )
			{
				if ( !src[x] )
				{
					dest[x] = dest[x] | mask;
				}
			}
		}
	}
	SelectObject( hdc, oldhdc ); //restore state
	DeleteObject( winBitmap );
	DeleteDC( hdc );
	return QPixmap::fromImage( image );
}
#endif //ifndef Q_OS_WINCE

// QFileIconProviderPrivate::getWinIcon copy modified to get icons for non-existing files
QIcon getWinIcon( const QFileInfo& fileInfo )
{
	QIcon retIcon;
	const QString fileExtension = QLatin1Char( '.' ) + fileInfo.suffix().toUpper();

	QString key;
	if ( fileInfo.isFile() && !fileInfo.isExecutable() && !fileInfo.isSymLink()
		 && fileExtension != QLatin1String( ".ICO" ) )
	{
		key = QLatin1String( "qt_" ) + fileExtension;
	}

	QPixmap pixmap;
	if ( !key.isEmpty() )
	{
		QPixmapCache::find( key, pixmap );
	}

	if ( !pixmap.isNull() )
	{
		retIcon.addPixmap( pixmap );
		if ( QPixmapCache::find( key + QLatin1Char( 'l' ), pixmap ) )
		{
			retIcon.addPixmap( pixmap );
		}
		return retIcon;
	}

	/* We don't use the variable, but by storing it statically, we
	 * ensure CoInitialize is only called once. */
	static HRESULT comInit = CoInitialize( NULL );
	Q_UNUSED( comInit );

	SHFILEINFO info;
	unsigned long val = 0;

	//Get the small icon
#ifndef Q_OS_WINCE
	val = SHGetFileInfo( ( const wchar_t* )QDir::toNativeSeparators( fileInfo.filePath() ).utf16(), 0, &info,
						 sizeof( SHFILEINFO ), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_ADDOVERLAYS | SHGFI_OVERLAYINDEX |
						 SHGFI_USEFILEATTRIBUTES );
#else
	val = SHGetFileInfo( ( const wchar_t* )QDir::toNativeSeparators( fileInfo.filePath() ).utf16(), 0, &info,
						 sizeof( SHFILEINFO ), SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES );
#endif

	// Even if GetFileInfo returns a valid result, hIcon can be empty in some cases
	if ( val && info.hIcon )
	{
		if ( fileInfo.isDir() && !fileInfo.isRoot() )
		{
			//using the unique icon index provided by windows save us from duplicate keys
			key = QString::fromLatin1( "qt_dir_%1" ).arg( info.iIcon );
			QPixmapCache::find( key, pixmap );
			if ( !pixmap.isNull() )
			{
				retIcon.addPixmap( pixmap );
				if ( QPixmapCache::find( key + QLatin1Char( 'l' ), pixmap ) )
				{
					retIcon.addPixmap( pixmap );
				}
				DestroyIcon( info.hIcon );
				return retIcon;
			}
		}
		if ( pixmap.isNull() )
		{
#ifndef Q_OS_WINCE
			pixmap = fromWinHICON( info.hIcon );
#else
			pixmap = fromWinHICON( ImageList_GetIcon( ( HIMAGELIST ) val, info.iIcon, ILD_NORMAL ) );
#endif
			if ( !pixmap.isNull() )
			{
				retIcon.addPixmap( pixmap );
				if ( !key.isEmpty() )
				{
					QPixmapCache::insert( key, pixmap );
				}
			}
			else
			{
				qWarning( "QFileIconProviderPrivate::getWinIcon() no small icon found" );
			}
		}
		DestroyIcon( info.hIcon );
	}

	//Get the big icon
#ifndef Q_OS_WINCE
	val = SHGetFileInfo( ( const wchar_t* )QDir::toNativeSeparators( fileInfo.filePath() ).utf16(), 0, &info,
						 sizeof( SHFILEINFO ), SHGFI_ICON | SHGFI_LARGEICON | SHGFI_SYSICONINDEX | SHGFI_ADDOVERLAYS | SHGFI_OVERLAYINDEX |
						 SHGFI_USEFILEATTRIBUTES );
#else
	val = SHGetFileInfo( ( const wchar_t* )QDir::toNativeSeparators( fileInfo.filePath() ).utf16(), 0, &info,
						 sizeof( SHFILEINFO ), SHGFI_LARGEICON | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES );
#endif
	if ( val && info.hIcon )
	{
		if ( fileInfo.isDir() && !fileInfo.isRoot() )
		{
			//using the unique icon index provided by windows save us from duplicate keys
			key = QString::fromLatin1( "qt_dir_%1" ).arg( info.iIcon );
		}
#ifndef Q_OS_WINCE
		pixmap = fromWinHICON( info.hIcon );
#else
		pixmap = fromWinHICON( ImageList_GetIcon( ( HIMAGELIST ) val, info.iIcon, ILD_NORMAL ) );
#endif
		if ( !pixmap.isNull() )
		{
			retIcon.addPixmap( pixmap );
			if ( !key.isEmpty() )
			{
				QPixmapCache::insert( key + QLatin1Char( 'l' ), pixmap );
			}
		}
		else
		{
			qWarning( "::getWinIcon() no large icon found" );
		}
		DestroyIcon( info.hIcon );
	}
	return retIcon;

}
#endif //Q_OS_WIN

FileIconProvider::FileIconProvider()
	: QFileIconProvider()
{
}

QIcon FileIconProvider::icon( const QFileInfo& info ) const
{
#ifdef Q_OS_WIN
	QIcon icon = getWinIcon( info );
	if ( !icon.isNull() )
	{
		return icon;
	}
#endif

	return QFileIconProvider::icon( info );
}

QIcon FileIconProvider::icon( const QString& file ) const
{
	QFileInfo info( file );
	return icon( info );
}
