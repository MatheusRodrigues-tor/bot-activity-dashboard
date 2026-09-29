#pragma once

#include <QObject>
#include <QFileSystemWatcher>
#include <QHash>
#include <QString>

// Espelha JSONs de referência do bot (catálogo de falhas conhecidas,
// tickets vinculados, assinaturas de falha recorrente) pra dentro da
// pasta servida pelo nginx, sempre que a pasta de origem mudar (ex:
// depois de um "git pull" no repo do bot, ou de o bot regravar seu
// estado). fileMap mapeia nome-de-origem -> nome-de-saída, permitindo
// instâncias diferentes pra fontes diferentes (o catálogo git-backed
// de referências e o estado local de assinaturas vivem em volumes
// Docker distintos).
class ReferenceCatalogSource : public QObject
{
    Q_OBJECT

public:
    ReferenceCatalogSource(const QString &sourceDir, const QString &outputDir,
                            const QHash<QString, QString> &fileMap,
                            QObject *parent = nullptr);

private slots:
    void mirrorNow();

private:
    QString m_sourceDir;
    QString m_outputDir;
    QHash<QString, QString> m_fileMap;
    QFileSystemWatcher m_watcher;
};
