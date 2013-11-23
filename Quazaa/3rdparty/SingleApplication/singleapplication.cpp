#include "singleapplication.h"

QString SingleApplication::ApplicationData::slogan("");
QByteArray SingleApplication::ApplicationData::encoding("ISO-8859-15");

/**
 * @brief SingleApplication::SingleApplication
 *  Constructor. Checks and fires up LocalServer or closes the program
 *  if another instance already exists
 * @param argc
 * @param argv
 */
SingleApplication::SingleApplication(int& argc, char* argv[]) :
  QApplication(argc, argv)
{
  _shouldContinue = false; // By default this is not the main process

  socket = new QLocalSocket();

  // Attempt to connect to the LocalServer
  socket->connectToServer("Quazaa");
  if(socket->waitForConnected(100)){
	socket->write("CMD:showUp");
	socket->flush();
	QThread::msleep(100);
	socket->close();
  } else {
	// The attempt was insuccessful, so we continue the program
	_shouldContinue = true;
	server = new LocalServer();
	server->start();
	QObject::connect(server, SIGNAL(showUp()), this, SLOT(slotShowUp()));
  }
}

QString SingleApplication::applicationSlogan()
{
	return ApplicationData::slogan;
}

void SingleApplication::setApplicationSlogan(const QString &slogan)
{
	ApplicationData::slogan = slogan;
}

QByteArray SingleApplication::encoding()
{
	return ApplicationData::encoding;
}

void SingleApplication::setEncoding(const QByteArray& encoding)
{
	ApplicationData::encoding = encoding;
}

/**
 * @brief SingleApplication::~SingleApplication
 *  Destructor
 */
SingleApplication::~SingleApplication()
{
  if(_shouldContinue){
	server->terminate();
  }
}

/**
 * @brief SingleApplication::shouldContinue
 *  Weather the program should be terminated
 * @return bool
 */
bool SingleApplication::shouldContinue()
{
  return _shouldContinue;
}

/**
 * @brief SingleApplication::slotShowUp
 *  Executed when the showUp command is sent to LocalServer
 */
void SingleApplication::slotShowUp()
{
  emit showUp();
}
