#include "costdatasource.h"

#include <QDir>
#include <QFile>
#include <QMap>
#include <QRegularExpression>
#include <QDebug>

namespace {
// Mesma convenção de nome de arquivo e mesma regex de custo usadas pelo
// post_cost_digest.py real do bot, pra bater com os números que ele posta
// no Slack.
const QRegularExpression kFilenameRe(QStringLiteral(R"(^(\d{8})-\d{6}_.*\.md$)"));
const QRegularExpression kCostRe(QStringLiteral(R"(^- Cost \(USD\): ([0-9.]+))"),
                                  QRegularExpression::MultilineOption);
}

CostDataSource::CostDataSource(const QString &reportsDir, QObject *parent)
    : QObject(parent), m_reportsDir(reportsDir)
{
    rescan();

    if (QDir(m_reportsDir).exists())
        m_watcher.addPath(m_reportsDir);
    else
        qWarning() << "Pasta de relatórios de custo não existe:" << m_reportsDir;

    connect(&m_watcher, &QFileSystemWatcher::directoryChanged,
            this, &CostDataSource::rescan);
}

QVariantList CostDataSource::costData() const
{
    return m_costData;
}

void CostDataSource::rescan()
{
    QDir dir(m_reportsDir);
    if (!dir.exists()) {
        qWarning() << "Pasta de relatórios de custo não existe:" << m_reportsDir;
        return;
    }

    QMap<QString, double> costByDate;
    QMap<QString, int> analysisCountByDate;

    const QStringList files = dir.entryList(QStringList() << "*.md", QDir::Files, QDir::Name);
    for (const QString &fileName : files) {
        const QRegularExpressionMatch nameMatch = kFilenameRe.match(fileName);
        if (!nameMatch.hasMatch())
            continue;

        const QString date = nameMatch.captured(1);

        QFile file(dir.filePath(fileName));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;

        const QString content = QString::fromUtf8(file.readAll());
        const QRegularExpressionMatch costMatch = kCostRe.match(content);
        if (costMatch.hasMatch()) {
            costByDate[date] += costMatch.captured(1).toDouble();
            analysisCountByDate[date] += 1;
        }
    }

    QVariantList list;
    for (auto it = costByDate.constBegin(); it != costByDate.constEnd(); ++it) {
        const QString &d = it.key();
        QVariantMap point;
        point[QStringLiteral("date")] =
            QStringLiteral("%1-%2-%3").arg(d.left(4), d.mid(4, 2), d.mid(6, 2));
        point[QStringLiteral("cost_usd")] = it.value();
        point[QStringLiteral("analysis_count")] = analysisCountByDate.value(d);
        list.append(point);
    }

    qInfo() << "CostDataSource: rescan encontrou" << list.size() << "dia(s) com custo em"
            << m_reportsDir;

    if (list != m_costData) {
        m_costData = list;
        emit costDataChanged();
    }
}
