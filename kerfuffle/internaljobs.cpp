/*
 * Copyright (c) 2007 Henrique Pinto <henrique.pinto@kdemail.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES ( INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION ) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * ( INCLUDING NEGLIGENCE OR OTHERWISE ) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "internaljobs.h"
#include <kdebug.h>


namespace Kerfuffle
{

	InternalListingJob::InternalListingJob( ReadOnlyArchiveInterface *archive, QObject *parent )
		: ThreadWeaver::Job( parent ), m_helper( 0 ), m_archive( archive ), m_success( false )
	{
	}

	InternalListingJob::~InternalListingJob()
	{
		delete m_helper;
		m_helper = 0;
	}

	void InternalListingJob::run()
	{
		m_helper = new ArchiveJobHelper( m_archive );
		connect( m_helper, SIGNAL( entry( const ArchiveEntry & ) ),
			 this, SIGNAL( entry( const ArchiveEntry & ) ) );
		connect( m_helper, SIGNAL( progress( double ) ),
			 this, SIGNAL( progress( double ) ) );
		connect( m_helper, SIGNAL( error( const QString&, const QString& ) ),
			 this, SIGNAL( error( const QString&, const QString& ) ) );
		m_success = m_helper->getTheListing();
	}

	InternalExtractJob::InternalExtractJob( ReadOnlyArchiveInterface *archive, const QList<QVariant> & files, const QString & destinationDirectory, bool preservePaths, QObject *parent )
		: ThreadWeaver::Job( parent ), m_archive( archive ), m_files( files ), m_destinationDirectory( destinationDirectory ),
		  m_helper( 0 ), m_success( false ), m_preservePaths( preservePaths )
	{

	}

	InternalExtractJob::~InternalExtractJob()
	{
		delete m_helper;
		m_helper = 0;
	}

	void InternalExtractJob::run()
	{
		m_helper = new ArchiveJobHelper( m_archive );
		connect( m_helper, SIGNAL( progress( double ) ),
			 this, SIGNAL( progress( double ) ) );
		connect( m_helper, SIGNAL( error( const QString&, const QString& ) ),
			 this, SIGNAL( error( const QString&, const QString& ) ) );
		m_archive->registerObserver( m_helper );
		m_success = m_archive->copyFiles( m_files, m_destinationDirectory, m_preservePaths );
		m_archive->removeObserver( m_helper );
	}

	InternalAddJob::InternalAddJob( ReadWriteArchiveInterface *archive, const QList<KUrl> & files, QObject *parent )
		: ThreadWeaver::Job( parent ), m_files( files ), m_archive( archive ), m_helper( 0 ), m_success( false )
	{
	}

	InternalAddJob::~InternalAddJob()
	{
		delete m_helper;
		m_helper = 0;
	}

	void InternalAddJob::run()
	{
		m_helper = new ArchiveJobHelper( m_archive );

		connect( m_helper, SIGNAL( entry( const ArchiveEntry & ) ),
			 this, SIGNAL( entry( const ArchiveEntry & ) ) );
		connect( m_helper, SIGNAL( progress( double ) ),
			 this, SIGNAL( progress( double ) ) );
		connect( m_helper, SIGNAL( error( const QString&, const QString& ) ),
			 this, SIGNAL( error( const QString&, const QString& ) ) );

		m_archive->registerObserver( m_helper );
		m_success = m_archive->addFiles( m_files );
		m_archive->removeObserver( m_helper );
	}

	ArchiveJobHelper::ArchiveJobHelper( ReadOnlyArchiveInterface *archive, QObject *parent )
		: QObject( parent ), m_archive( archive )
	{
	}

	ArchiveJobHelper::~ArchiveJobHelper()
	{
	}

	bool ArchiveJobHelper::getTheListing()
	{
		m_archive->registerObserver( this );
		bool result = m_archive->list();
		m_archive->removeObserver( this );
		return result;
	}

	void ArchiveJobHelper::onError( const QString & message, const QString & details )
	{
		emit error( message, details );
	}

	void ArchiveJobHelper::onEntry( const ArchiveEntry & archiveEntry )
	{
		emit entry( archiveEntry );
	}

	void ArchiveJobHelper::onProgress( double d )
	{
		emit progress( d );
	}


	void ArchiveJobHelper::entryslot( const ArchiveEntry & e )
	{
		kDebug( 1601 ) << k_funcinfo << "Entry: " << e[ FileName ] << ", Owner = " << e[ Owner ] << endl;
	}

} // namespace Kerfuffle

#include "internaljobs.moc"