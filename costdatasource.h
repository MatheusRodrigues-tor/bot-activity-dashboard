#pragma once

#include <QObject>
#include <QVariantList>
#include <QFileSystemWatcher>

// Observa a pasta de relatórios de custo do bot Sherlog Builds
// (arquivos "YYYYMMDD-HHMMSS_<id>_<status>.md") e expõe o custo total por
// dia como uma lista pro QML consumir. Atualiza sozinha sempre que um novo
// relatório aparece na pasta (o bot só gera um quando o Claude é invocado).
class CostDataSource : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList costData READ costData NOTIFY costDataChanged)

public:
    explicit CostDataSource(const QString &reportsDir, QObject *parent = nullptr);

    QVariantList costData() const;

signals:
    void costDataChanged();

private slots:
    void rescan();

private:
    QString m_reportsDir;
    QVariantList m_costData;
    QFileSystemWatcher m_watcher;
};
