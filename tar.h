#ifndef TAR_H
#define TAR_H

#include <unistd.h>

// Qt includes
#include <qdir.h>
#include <qobject.h>
#include <qstring.h>
#include <qstrlist.h>


// KDE includes
#include <kprocess.h>

// ark includes

#include "arch.h"
#include "arkdata.h"
#include "filelistview.h"


class ArkWidget;

class TarArch : public QObject, public Arch {

Q_OBJECT

public slots:
	void inputPending( KProcess *, char *buffer, int bufflen );  
	void updateExtractProgress( KProcess *, char *buffer, int bufflen );
	void openFinished( KProcess * );
	void updateFinished( KProcess * );
	void createTmpFinished( KProcess * );
	void extractFinished( KProcess * );

public:
	TarArch( ArkData*, ArkWidget*, FileListView* );
	virtual ~TarArch();
	
	/*virtual*/ unsigned char setOptions( bool p, bool l, bool o );
	
	virtual void openArch( QString );
	virtual void createArch( QString );
	
	virtual int addFile( QStrList *);
	virtual void deleteSelectedFiles();
	virtual void deleteFiles( const QString& );
	virtual void extractTo( QString );
	virtual void extraction();
	
	const QStrList *getListing();
	virtual QString unarchFile( int, QString );
	
	virtual int getEditFlag();
	
	QString getCompressor();
	QString getUnCompressor();

private:
	char          *stdout_buf;
	QStrList      *listing;
	QString       tmpfile;
	bool          compressed;
	ArkData	      *m_data;
	ArkWidget     *m_arkwidget;
	KProcess      kproc;
	FileListView  *destination_flw;

	bool          perms, tolower, overwrite;
	int           updateArch();
	void          createTmp();
};

#endif /* TAR_H */
