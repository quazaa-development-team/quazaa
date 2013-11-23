#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#include "localserver.h"

#include <QApplication>
#include <QLocalSocket>
#include <QString>
#include <QByteArray>

/**
 * @brief The Application class handles trivial application initialization procedures
 */
class SingleApplication : public QApplication
{
	Q_OBJECT

public:
	SingleApplication(int& argc, char* argv[]);
	~SingleApplication();
	bool shouldContinue();

	static QString applicationSlogan();
	static void setApplicationSlogan(const QString& slogan);

	static QByteArray encoding();
	static void setEncoding(const QByteArray& encoding);

signals:
	void showUp();

private slots:
	void slotShowUp();

private:
	QLocalSocket* socket;
	LocalServer* server;
	bool _shouldContinue;
	struct ApplicationData {
		static QString slogan;
		static QByteArray encoding;
	};

};

#endif // SINGLEAPPLICATION_H
