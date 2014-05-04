/*
** $Id$
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

#include "chatconverter.h"

#include <QTextDocument>
#include <QTextBlock>
#include <QTextFragment>
#include <QTextCharFormat>

#include <QDebug>

#ifdef _DEBUG
#include "debug_new.h"
#endif

ChatConverter::ChatConverter( QTextDocument* pDoc ) :
	m_pDocument( pDoc )
{
}
ChatConverter::~ChatConverter()
{

}
QString ChatConverter::toHtml()
{
	m_nOutputFormat = HTML;
	m_pResult.clear();
	m_pStack.clear();

	QTextBlock currentBlock = m_pDocument->begin();

	while ( currentBlock.isValid() )
	{
		processBlock( &currentBlock );
		currentBlock = currentBlock.next();
	}

	return m_pResult;
}
QString ChatConverter::toBBCode()
{
	m_nOutputFormat = BBCODE;
	m_pResult.clear();
	m_pStack.clear();

	QTextBlock currentBlock = m_pDocument->begin();

	while ( currentBlock.isValid() )
	{
		processBlock( &currentBlock );
		currentBlock = currentBlock.next();
	}

	return m_pResult;
}
QString ChatConverter::toIrc()
{
	m_nOutputFormat = IRCCODE;
	m_pResult.clear();
	m_pStack.clear();

	QTextBlock currentBlock = m_pDocument->begin();

	while ( currentBlock.isValid() )
	{
		processBlock( &currentBlock );
		currentBlock = currentBlock.next();
	}

	return m_pResult;
}

void ChatConverter::processBlock( QTextBlock* pBlock )
{
	QTextBlock::iterator it;
	for ( it = pBlock->begin(); !( it.atEnd() ); ++it )
	{
		QTextFragment currentFragment = it.fragment();
		if ( currentFragment.isValid() )
		{
			switch ( m_nOutputFormat )
			{
			case HTML:
				processFragment( &currentFragment );
				break;
			case BBCODE:
				processFragmentBB( &currentFragment );
				break;
			case IRCCODE:
				processFragmentIRC( &currentFragment );
			}
		}
	}

	while ( !m_pStack.isEmpty() )
	{
		QString sTag = m_pStack.pop();
		switch ( m_nOutputFormat )
		{
		case HTML:
			m_pResult.append( "</" + sTag + ">" );
			break;
		case BBCODE:
			m_pResult.append( "[/" + sTag + "]" );
			break;
		case IRCCODE:
			m_pResult.append( sTag );
			break;
		}
	}
}
void ChatConverter::processFragment( QTextFragment* pFrag )
{
	QTextCharFormat fmt = pFrag->charFormat();

	bool bLoop = true;
	while ( bLoop && !m_pStack.isEmpty() )
	{
		bLoop = false;

		if ( m_pStack.top() == "b" && fmt.fontWeight() <= QFont::Normal )
		{
			m_pResult.append( "</b>" );
			bLoop = true;
			m_pStack.pop();
		}
		else if ( m_pStack.top() == "i" && !fmt.fontItalic() )
		{
			m_pResult.append( "</i>" );
			bLoop = true;
			m_pStack.pop();
		}
		else if ( m_pStack.top() == "u" && !fmt.fontUnderline() )
		{
			m_pResult.append( "</u>" );
			bLoop = true;
			m_pStack.pop();
		}

	}

	if ( fmt.fontWeight() > QFont::Normal && !m_pStack.contains( "b" ) )
	{
		m_pResult.append( "<b>" );
		m_pStack.push( "b" );
	}

	if ( fmt.fontItalic() && !m_pStack.contains( "i" ) )
	{
		m_pResult.append( "<i>" );
		m_pStack.push( "i" );
	}

	if ( fmt.fontUnderline() && !m_pStack.contains( "u" ) )
	{
		m_pResult.append( "<u>" );
		m_pStack.push( "u" );
	}

	m_pResult.append( pFrag->text() );
}

void ChatConverter::processFragmentBB( QTextFragment* pFrag )
{
	QTextCharFormat fmt = pFrag->charFormat();

	bool bLoop = true;
	while ( bLoop && !m_pStack.isEmpty() )
	{
		bLoop = false;

		if ( m_pStack.top() == "b" && fmt.fontWeight() <= QFont::Normal )
		{
			m_pResult.append( "[/b]" );
			bLoop = true;
			m_pStack.pop();
		}
		else if ( m_pStack.top() == "i" && !fmt.fontItalic() )
		{
			m_pResult.append( "[/i]" );
			bLoop = true;
			m_pStack.pop();
		}
		else if ( m_pStack.top() == "u" && !fmt.fontUnderline() )
		{
			m_pResult.append( "[/u]" );
			bLoop = true;
			m_pStack.pop();
		}

	}

	if ( fmt.fontWeight() > QFont::Normal && !m_pStack.contains( "b" ) )
	{
		m_pResult.append( "[b]" );
		m_pStack.push( "b" );
	}

	if ( fmt.fontItalic() && !m_pStack.contains( "i" ) )
	{
		m_pResult.append( "[i]" );
		m_pStack.push( "i" );
	}

	if ( fmt.fontUnderline() && !m_pStack.contains( "u" ) )
	{
		m_pResult.append( "[u]" );
		m_pStack.push( "u" );
	}

	m_pResult.append( pFrag->text() );
}

void ChatConverter::processFragmentIRC( QTextFragment* pFrag )
{
	QTextCharFormat fmt = pFrag->charFormat();

	bool bLoop = true;
	while ( bLoop && !m_pStack.isEmpty() )
	{
		bLoop = false;

		if ( m_pStack.top() == QString( IrcFormatCodes::Bold ) && fmt.fontWeight() <= QFont::Normal )
		{
			m_pResult.append( IrcFormatCodes::Bold );
			bLoop = true;
			m_pStack.pop();
		}
		else if ( m_pStack.top() == QString( IrcFormatCodes::Italic ) && !fmt.fontItalic() )
		{
			m_pResult.append( IrcFormatCodes::Italic );
			bLoop = true;
			m_pStack.pop();
		}
		else if ( m_pStack.top() == QString( IrcFormatCodes::Underline ) && !fmt.fontUnderline() )
		{
			m_pResult.append( IrcFormatCodes::Underline );
			bLoop = true;
			m_pStack.pop();
		}
		else if ( m_pStack.top() == "2" && ( fmt.foreground().color() != QColor( "darkblue" ) ) )
		{
			m_pResult.append( "\017" );
			bLoop = true;
			m_pStack.pop();
		}
		else if ( m_pStack.top() == "3" && ( fmt.foreground().color() != QColor( "darkgreen" ) ) )
		{
			m_pResult.append( "\017" );
			bLoop = true;
			m_pStack.pop();
		}
		else if ( m_pStack.top() == "4" && ( fmt.foreground().color() != QColor( "red" ) ) )
		{
			m_pResult.append( "\017" );
			bLoop = true;
			m_pStack.pop();
		}
		else if ( m_pStack.top() == "5" && ( fmt.foreground().color() != QColor( "darkred" ) ) )
		{
			m_pResult.append( "\017" );
			bLoop = true;
			m_pStack.pop();
		}
		else if ( m_pStack.top() == "6" && ( fmt.foreground().color() != QColor( "darkviolet" ) ) )
		{
			m_pResult.append( "\017" );
			bLoop = true;
			m_pStack.pop();
		}
		else if ( m_pStack.top() == "7" && ( fmt.foreground().color() != QColor( "orange" ) ) )
		{
			m_pResult.append( "\017" );
			bLoop = true;
			m_pStack.pop();
		}
		else if ( m_pStack.top() == "8" && ( fmt.foreground().color() != QColor( "yellow" ) ) )
		{
			m_pResult.append( "\017" );
			bLoop = true;
			m_pStack.pop();
		}
		else if ( m_pStack.top() == "9" && ( fmt.foreground().color() != QColor( "lightgreen" ) ) )
		{
			m_pResult.append( "\017" );
			bLoop = true;
			m_pStack.pop();
		}
		else if ( m_pStack.top() == "10" && ( fmt.foreground().color() != QColor( "cornflowerblue" ) ) )
		{
			m_pResult.append( "\017" );
			bLoop = true;
			m_pStack.pop();
		}
		else if ( m_pStack.top() == "11" && ( fmt.foreground().color() != QColor( "lightblue" ) ) )
		{
			m_pResult.append( "\017" );
			bLoop = true;
			m_pStack.pop();
		}
		else if ( m_pStack.top() == "12" && ( fmt.foreground().color() != QColor( "blue" ) ) )
		{
			m_pResult.append( "\017" );
			bLoop = true;
			m_pStack.pop();
		}
		else if ( m_pStack.top() == "13" && ( fmt.foreground().color() != QColor( "violet" ) ) )
		{
			m_pResult.append( "\017" );
			bLoop = true;
			m_pStack.pop();
		}
		else if ( m_pStack.top() == "14" && ( fmt.foreground().color() != QColor( "gray" ) ) )
		{
			m_pResult.append( "\017" );
			bLoop = true;
			m_pStack.pop();
		}
		else if ( m_pStack.top() == "15" && ( fmt.foreground().color() != QColor( "darkgray" ) ) )
		{
			m_pResult.append( "\017" );
			bLoop = true;
			m_pStack.pop();
		}

	}

	if ( fmt.fontWeight() > QFont::Normal && !m_pStack.contains( QString( IrcFormatCodes::Bold ) ) )
	{
		m_pResult.append( IrcFormatCodes::Bold );
		m_pStack.push( QString( IrcFormatCodes::Bold ) );
	}

	if ( fmt.fontItalic() && !m_pStack.contains( QString( IrcFormatCodes::Italic ) ) )
	{
		m_pResult.append( IrcFormatCodes::Italic );
		m_pStack.push( QString( IrcFormatCodes::Italic ) );
	}

	if ( fmt.fontUnderline() && !m_pStack.contains( QString( IrcFormatCodes::Underline ) ) )
	{
		m_pResult.append( IrcFormatCodes::Underline );
		m_pStack.push( QString( IrcFormatCodes::Underline ) );
	}
	else if ( ( fmt.foreground().color() == QColor( "darkblue" ) ) && !m_pStack.contains( "2" ) )
	{
		m_pResult.append( "2" );
		m_pStack.push( "2" );
	}
	else if ( ( fmt.foreground().color() == QColor( "darkgreen" ) ) && !m_pStack.contains( "3" ) )
	{
		m_pResult.append( "3" );
		m_pStack.push( "3" );
	}
	else if ( ( fmt.foreground().color() == QColor( "red" ) ) && !m_pStack.contains( "4" ) )
	{
		m_pResult.append( "4" );
		m_pStack.push( "4" );
	}
	else if ( ( fmt.foreground().color() == QColor( "darkred" ) ) && !m_pStack.contains( "5" ) )
	{
		m_pResult.append( "5" );
		m_pStack.push( "5" );
	}
	else if ( ( fmt.foreground().color() == QColor( "darkviolet" ) ) && !m_pStack.contains( "6" ) )
	{
		m_pResult.append( "6" );
		m_pStack.push( "6" );
	}
	else if ( ( fmt.foreground().color() == QColor( "orange" ) ) && !m_pStack.contains( "7" ) )
	{
		m_pResult.append( "7" );
		m_pStack.push( "7" );
	}
	else if ( ( fmt.foreground().color() == QColor( "yellow" ) ) && !m_pStack.contains( "8" ) )
	{
		m_pResult.append( "8" );
		m_pStack.push( "8" );
	}
	else if ( ( fmt.foreground().color() == QColor( "lightgreen" ) ) && !m_pStack.contains( "9" ) )
	{
		m_pResult.append( "9" );
		m_pStack.push( "9" );
	}
	else if ( ( fmt.foreground().color() == QColor( "cornflowerblue" ) ) && !m_pStack.contains( "10" ) )
	{
		m_pResult.append( "10" );
		m_pStack.push( "10" );
	}
	else if ( ( fmt.foreground().color() == QColor( "lightblue" ) ) && !m_pStack.contains( "11" ) )
	{
		m_pResult.append( "11" );
		m_pStack.push( "11" );
	}
	else if ( ( fmt.foreground().color() == QColor( "blue" ) ) && !m_pStack.contains( "12" ) )
	{
		m_pResult.append( "12" );
		m_pStack.push( "12" );
	}
	else if ( ( fmt.foreground().color() == QColor( "violet" ) ) && !m_pStack.contains( "13" ) )
	{
		m_pResult.append( "13" );
		m_pStack.push( "13" );
	}
	else if ( ( fmt.foreground().color() == QColor( "gray" ) ) && !m_pStack.contains( "14" ) )
	{
		m_pResult.append( "14" );
		m_pStack.push( "14" );
	}
	else if ( ( fmt.foreground().color() == QColor( "darkgray" ) ) && !m_pStack.contains( "15" ) )
	{
		m_pResult.append( "15" );
		m_pStack.push( "15" );
	}

	m_pResult.append( pFrag->text() );
}

