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

    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this,
            &ReferenceCatalogSource::mirrorNow);
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
}
