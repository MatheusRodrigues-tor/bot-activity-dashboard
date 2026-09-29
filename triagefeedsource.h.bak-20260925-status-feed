#pragma once

#include <QObject>
#include <QVariantList>
#include <QFileSystemWatcher>

// Lista bruta (um item por evento, não agregada por dia) dos relatórios de
// triagem mais recentes, pra mostrar "builds que falharam" com status e
// resumo do Jenkins. Observa a mesma pasta que o CostDataSource.
class TriageFeedSource : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList feed READ feed NOTIFY feedChanged)

public:
    explicit TriageFeedSource(const QString &reportsDir, QObject *parent = nullptr);

    QVariantList feed() const;

signals:
    void feedChanged();

private slots:
    void rescan();

private:
    QString m_reportsDir;
    QVariantList m_feed;
    QFileSystemWatcher m_watcher;
};
