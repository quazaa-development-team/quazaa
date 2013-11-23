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


#ifndef HUBHORIZON_H
#define HUBHORIZON_H

#include "types.h"

class G2Packet;


class CHubHorizonHub
{
public:
	CEndPoint		m_oAddress;
	quint32			m_nReference;
	CHubHorizonHub*	m_pNext;
};


class CHubHorizonGroup
{
public:
	CHubHorizonGroup();
	virtual ~CHubHorizonGroup();

protected:
	CHubHorizonHub**	m_pList;
	quint32				m_nCount;
	quint32				m_nBuffer;

public:
	void		add(CEndPoint oAddress);
	void		clear();

};


class CHubHorizonPool
{
public:
	CHubHorizonPool();
	virtual ~CHubHorizonPool();

protected:
	CHubHorizonHub*		m_pBuffer;
	quint32				m_nBuffer;
	CHubHorizonHub*		m_pFree;
	CHubHorizonHub*		m_pActive;
	quint32				m_nActive;

public:
	void				setup();
	void				clear();
	CHubHorizonHub*		add(CEndPoint oAddress);
	void				remove(CHubHorizonHub* pHub);
	CHubHorizonHub*		find(CEndPoint oAddress);
	int					addHorizonHubs(G2Packet* pPacket);

};

extern CHubHorizonPool	HubHorizonPool;


#endif // HUBHORIZON_H
