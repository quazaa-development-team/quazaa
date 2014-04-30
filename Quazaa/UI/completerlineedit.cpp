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

#include "completerlineedit.h"

CCompleterLineEdit::CCompleterLineEdit( const QStringList& completions, QWidget* parent ) :
	QLineEdit( parent )
{
	m_pCompleter = new QCompleter( completions, this );
	setCaseSensitivity( Qt::CaseInsensitive );
	setCompletionMode( QCompleter::UnfilteredPopupCompletion );
	setCompleter( m_pCompleter );
	m_bCompleteOnDoubleClick = true;
}

void CCompleterLineEdit::mouseDoubleClickEvent( QMouseEvent* )
{
	m_pCompleter->complete();
}

void CCompleterLineEdit::setCaseSensitivity( Qt::CaseSensitivity caseSensitivity )
{
	m_pCompleter->setCaseSensitivity( caseSensitivity );
}

Qt::CaseSensitivity CCompleterLineEdit::caseSensitivity() const
{
	return m_pCompleter->caseSensitivity();
}

void CCompleterLineEdit::setCompletionMode( QCompleter::CompletionMode mode )
{
	m_pCompleter->setCompletionMode( mode );
}

QCompleter::CompletionMode CCompleterLineEdit::completionMode() const
{
	return m_pCompleter->completionMode();
}

void CCompleterLineEdit::setWrapAround( bool wrap )
{
	m_pCompleter->setWrapAround( wrap );
}

bool CCompleterLineEdit::wrapAround() const
{
	return m_pCompleter->wrapAround();
}

bool CCompleterLineEdit::completeOnDoubleClick() const
{
	return m_bCompleteOnDoubleClick;
}

void CCompleterLineEdit::setCompleteOnDoubleClick( bool complete )
{
	m_bCompleteOnDoubleClick = complete;
}
