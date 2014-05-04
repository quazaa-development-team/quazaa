#ifndef METALINK4HANDLER_H
#define METALINK4HANDLER_H

#include <QFile>

#include "download.h"
#include "metalinkhandler.h"

namespace URI
{

class Metalink4Handler : public MetalinkHandler
{
public:
	explicit Metalink4Handler( QFile& oFile = m_oEmptyQFile );

	Download* file( const unsigned int& ID ) const;

private:
	bool parseFile( QList<MetaFile>& lFiles, quint16 ID );
};

} // namespace URI

#endif // METALINK4HANDLER_H
