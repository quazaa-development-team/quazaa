/*
** gwc.h
**
** Copyright © Quazaa Development Team, 2012.
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

#ifndef GWC_H
#define GWC_H

#include "discoveryservice.h"

namespace Discovery
{

// http://www.gnucleus.com/gwebcache/newgwc.html

/**
 * @brief The CGWC class implements a Gnutella Web Cache compatible to the GWC spec 2.0.
 */
class GWC : public DiscoveryService
{
	Q_OBJECT

	/* ========================================================================================== */
	/* ======================================= Attributes ======================================= */
	/* ========================================================================================== */
private:
	QNAMPtr m_pNAMgr;
	QNetworkRequest* m_pRequest;
	bool m_bGnutella;
	bool m_bG2;

	/* ========================================================================================== */
	/* ====================================== Construction ====================================== */
	/* ========================================================================================== */
public:
	GWC( const QUrl& oURL, const NetworkType& oNType, quint8 nRating );

	~GWC();

	/* ========================================================================================== */
	/* ======================================= Operations ======================================= */
	/* ========================================================================================== */
	QString     type()       const;
	inline bool isGnutella() const
	{
		return m_bGnutella;
	}
	inline bool isG2()       const
	{
		return m_bG2;
	}

private:
	void doQuery()  throw();
	void doUpdate() throw();
	void doCancelRequest() throw();

private slots:
	void requestCompleted( QNetworkReply* pReply );
};

}

#endif // GWC_H
