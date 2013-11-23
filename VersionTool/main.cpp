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

// Original credit goes to Rob Caldecott
// http://qtcreator.blogspot.com/

#include <iostream>
#include <QProcess>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QDate>
#include <QTime>
#include <QFileInfo>
#include <QTemporaryFile>
#include <cstdlib>

using namespace std;

static QString getBuildNumber()
{
  const QDateTime today(QDateTime::currentDateTimeUtc());
  return (today.date().toString("yyyy-MM-dd"));
}

static QString getGitShortVersionHash()
{
  QProcess process;
  bool result;
  process.start("git rev-parse --short HEAD");
  result = process.waitForFinished(1000);

  if (result)
	return process.readAllStandardOutput().trimmed();
  else
	  return "unknown";
}

static QByteArray readFile(const QString& fileName)
{
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly))
  {
	return QByteArray();
  }
  return file.readAll();
}

static int writeFile(const QString& fileName, const int major, const int minor, const QString revision, const QString build)
{
  // Create a temp file containing the version info and
  // only replace the existing one if they are different
  QTemporaryFile tempFile;
  if (tempFile.open())
  {
	QTextStream out(&tempFile);
	out << "#ifndef VERSION_H\r\n";
	out << "#define VERSION_H\r\n\r\n";
	out << "namespace Version\r\n";
	out << "{\r\n";
	out << "\tstatic const int MAJOR = " << major << ";\r\n";
	out << "\tstatic const int MINOR = " << minor << ";\r\n";

// Note: this only works for newer versions of gcc/MinGW
	out << "#pragma GCC diagnostic push\r\n";                         // save diagnostic settings
	out << "#pragma GCC diagnostic ignored \"-Wunused-variable\"\r\n";// suppress unused var warning
	out << "\tstatic const char* REVISION = \"" << revision << "\";\r\n";
	out << "\tstatic const char* BUILD_DATE = \"" << build << "\";\r\n";
	out << "#pragma GCC diagnostic pop\r\n";                          // restore diagnostic settings
	out << "}\r\n\r\n";
	out << "#endif // VERSION_H\r\n";

	const QString tempFileName = tempFile.fileName();
	tempFile.close();

	if (!QFile::exists(fileName) || readFile(fileName) != readFile(tempFileName))
	{
	  QFile::remove(fileName);
	  QFile::copy(tempFileName, fileName);
	}

	return 0;
  }
  else
  {
	cout << "Error creating temporary file!" << endl;
	return 1;
  }
}

int main(int argc, char *argv[])
{
  if (argc != 4)
  {
	cout << "Usage: version major minor filename" << endl;
	return 1;
  }

  const int major = atoi(argv[1]);
  const int minor = atoi(argv[2]);
  const QString revision = getGitShortVersionHash();
  const QString build = getBuildNumber();

  cout << major << '.' << minor << '.' << revision.toLocal8Bit().constData() << ' ' << build.toLocal8Bit().constData() << endl;

  return writeFile(argv[3], major, minor, revision, build);
}
