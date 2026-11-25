#include "referencecatalogsource.h"

#include <QDir>
#include <QFile>
#include <QDebug>

ReferenceCatalogSource::ReferenceCatalogSource(const QString &sourceDir, const QString &outputDir,
                                                const QHash<QString, QString> &fileMap,
                                                QObject *parent)
    : QObject(parent), m_sourceDir(sourceDir), m_outputDir(outputDir), m_fileMap(fileMap)
{
    mirrorNow();

    if (QDir(m_sourceDir).exists())
        m_watcher.addPath(m_sourceDir);
    else
        qWarning() << "Pasta de referências não existe:" << m_sourceDir;

    // Watching the directory alone only reliably fires for files being
    // added/removed - confirmed in practice: a script rewriting
    // failure_signatures.json in place (sync_jira_ticket_status.py, see
    // the bot repo) with nothing else in the directory changing at the
    // same moment left the dashboard showing a stale ticket_status until
    // manually restarted. Watching each individual file too, via
    // fileChanged, catches a pure in-place content overwrite directly.
    watchIndividualFiles();

    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this,
            &ReferenceCatalogSource::mirrorNow);
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this,
            &ReferenceCatalogSource::mirrorNow);
}

void ReferenceCatalogSource::watchIndividualFiles()
{
    for (auto it = m_fileMap.constBegin(); it != m_fileMap.constEnd(); ++it) {
        const QString path = m_sourceDir + "/" + it.key();
        if (QFile::exists(path) && !m_watcher.files().contains(path))
            m_watcher.addPath(path);
    }
}

void ReferenceCatalogSource::mirrorNow()
{
    for (auto it = m_fileMap.constBegin(); it != m_fileMap.constEnd(); ++it) {
        QFile in(m_sourceDir + "/" + it.key());
        if (!in.open(QIODevice::ReadOnly))
            continue;
        QFile out(m_outputDir + "/" + it.value());
        if (out.open(QIODevice::WriteOnly | QIODevice::Truncate))
            out.write(in.readAll());
    }
    // Some writers replace a file via write-to-temp-then-rename (not an
    // in-place write), which silently drops the inotify watch tied to the
    // old inode even though the path keeps existing - re-checking after
    // every mirror re-establishes it when that happens, and is a no-op
    // otherwise (watchIndividualFiles skips paths already watched).
    watchIndividualFiles();
}
