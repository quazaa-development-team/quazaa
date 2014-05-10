#include "wizardircconnection.h"
#include "ui_wizardircconnection.h"

#include "quazaasettings.h"

CWizardIrcConnection::CWizardIrcConnection( QWidget* parent ) :
	QWizard( parent ),
	ui( new Ui::CWizardIrcConnection )
{
	ui->setupUi( this );
	setPixmap( QWizard::LogoPixmap, QPixmap( ":/Resource/Chat/Chat.png" ) );
	setWindowFilePath( tr( "Connection" ) );

	quazaaSettings.loadChatConnectionWizard();
	//Users page
	lineEditNickName = new CCompleterLineEdit( quazaaSettings.Chat.NickNames, this );
	QRegularExpressionValidator* validator =
			new QRegularExpressionValidator( QRegularExpression( "\\S+" ), lineEditNickName );
	lineEditNickName->setValidator( validator );
	ui->horizontalLayoutNick->addWidget( lineEditNickName );
	connect( lineEditNickName, SIGNAL( textChanged( QString ) ), this, SLOT( completeChanged() ) );
	ui->labelNick->setBuddy( lineEditNickName );

	lineEditRealName = new CCompleterLineEdit( quazaaSettings.Chat.RealNames, this );
	ui->horizontalLayoutReal->addWidget( lineEditRealName );
	ui->labelReal->setBuddy( lineEditRealName );

	// Server page
	connect( ui->spinBoxPort, SIGNAL( valueChanged( int ) ), this, SLOT( completeChanged() ) );

	lineEditHost = new CCompleterLineEdit( quazaaSettings.Chat.Hosts, this );
	ui->horizontalLayoutHost->addWidget( lineEditHost );
	connect( lineEditHost, SIGNAL( textChanged( QString ) ), this, SLOT( completeChanged() ) );
	ui->labelHost->setBuddy( lineEditHost );

	lineEditUserName = new CCompleterLineEdit( quazaaSettings.Chat.UserNames, this );
	ui->horizontalLayoutUserName->addWidget( lineEditUserName );
	connect( lineEditUserName, SIGNAL( textChanged( QString ) ), this, SLOT( completeChanged() ) );
	ui->labelUser->setBuddy( lineEditUserName );

	// Connection page
	lineEditConnectionName = new CCompleterLineEdit( quazaaSettings.Chat.ConnectionNames, this );
	ui->horizontalLayoutConnectionName->addWidget( lineEditConnectionName );
	ui->labelConnectionName->setBuddy( lineEditConnectionName );

	connect( this, SIGNAL( currentIdChanged( int ) ), this, SLOT( completeChanged() ) );
	button( QWizard::NextButton )->setEnabled( false );
}

CWizardIrcConnection::~CWizardIrcConnection()
{
	delete ui;
}

void CWizardIrcConnection::accept()
{
	if ( !quazaaSettings.Chat.NickNames.contains( nickName(), Qt::CaseInsensitive ) )
	{
		quazaaSettings.Chat.NickNames << nickName();
	}
	if ( !quazaaSettings.Chat.RealNames.contains( realName(), Qt::CaseInsensitive ) )
	{
		quazaaSettings.Chat.RealNames << realName();
	}
	if ( !quazaaSettings.Chat.Hosts.contains( hostName(), Qt::CaseInsensitive ) )
	{
		quazaaSettings.Chat.Hosts << hostName();
	}
	if ( !quazaaSettings.Chat.UserNames.contains( userName(), Qt::CaseInsensitive ) )
	{
		quazaaSettings.Chat.UserNames << userName();
	}
	if ( !quazaaSettings.Chat.ConnectionNames.contains( connectionName(), Qt::CaseInsensitive ) )
	{
		quazaaSettings.Chat.ConnectionNames << connectionName();
	}
	quazaaSettings.saveChatConnectionWizard();

	QDialog::accept();
}

ConnectionInfo CWizardIrcConnection::connection() const
{
	ConnectionInfo connection;
	connection.nick = nickName();
	connection.real = realName();
	connection.host = hostName();
	connection.port = port();
	connection.secure = isSecure();
	connection.user = userName();
	connection.pass = password();
	connection.name = connectionName();
	return connection;
}

void CWizardIrcConnection::setConnection( const ConnectionInfo& connection )
{
	if ( !connection.nick.isEmpty() )
	{
		setNickName( connection.nick );
	}
	else
	{
		setNickName( quazaaSettings.Profile.IrcNickname );
	}

	if ( !connection.real.isEmpty() )
	{
		setRealName( connection.real );
	}
	else
	{
		setRealName( quazaaSettings.Profile.IrcUserName );
	}

	if ( !connection.host.isEmpty() )
	{
		setHostName( connection.host );
	}
	else
	{
		setHostName( "irc.paradoxirc.net" );
	}

	if ( connection.port )
	{
		setPort( connection.port );
	}
	else
	{
		setPort( 6667 );
	}

	setSecure( connection.secure );
	setUserName( connection.user );
	setPassword( connection.pass );
	setConnectionName( connection.name );
}

QString CWizardIrcConnection::nickName() const
{
	return lineEditNickName->text();
}

void CWizardIrcConnection::setNickName( const QString& nickName )
{
	lineEditNickName->setText( nickName );
}

QString CWizardIrcConnection::realName() const
{
	return lineEditRealName->text();
}

void CWizardIrcConnection::setRealName( const QString& realName )
{
	lineEditRealName->setText( realName );
}

QString CWizardIrcConnection::hostName() const
{
	return lineEditHost->text();
}

void CWizardIrcConnection::setHostName( const QString& hostName )
{
	lineEditHost->setText( hostName );
}

quint16 CWizardIrcConnection::port() const
{
	return ui->spinBoxPort->value();
}

void CWizardIrcConnection::setPort( quint16 port )
{
	ui->spinBoxPort->setValue( port );
}

bool CWizardIrcConnection::isSecure() const
{
	return ui->checkBoxSecure->isChecked();
}

void CWizardIrcConnection::setSecure( bool secure )
{
	ui->checkBoxSecure->setChecked( secure );
}

QString CWizardIrcConnection::userName() const
{
	return lineEditUserName->text();
}

void CWizardIrcConnection::setUserName( const QString& userName )
{
	lineEditUserName->setText( userName );
}

QString CWizardIrcConnection::password() const
{
	return ui->lineEditPassword->text();
}

void CWizardIrcConnection::setPassword( const QString& password )
{
	ui->lineEditPassword->setText( password );
}

QString CWizardIrcConnection::connectionName() const
{
	return lineEditConnectionName->text();
}

void CWizardIrcConnection::setConnectionName( const QString& name )
{
	lineEditConnectionName->setText( name );
}

bool CWizardIrcConnection::isComplete() const
{
	if ( currentPage() == ui->wizardPageUser )
	{
		return !lineEditNickName->text().isEmpty();
	}
	else if ( currentPage() == ui->wizardPageServer )
	{
		return !lineEditHost->text().isEmpty() && ui->spinBoxPort->hasAcceptableInput();
	}
	else
	{
		return true;
	}
}

void CWizardIrcConnection::completeChanged()
{
	button( QWizard::NextButton )->setEnabled( isComplete() );
}
