#include "triagefeedsource.h"

#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QDebug>
#include <algorithm>

namespace {
// Mesma convenção de nome de arquivo do CostDataSource, mas aqui capturamos
// também a hora, o id do evento e o status. Status allows dashes+digits, not
// just [a-z]+ - _write_deterministic_report emits
// "deterministic-<known-failures-entry-id>" (e.g.
// "deterministic-repo-sync-rebase-conflict-2"), which a letters-only class
// would silently fail to match, dropping the whole report from the feed.
const QRegularExpression kFilenameRe(
    QStringLiteral(R"(^(\d{8})-(\d{6})_(.+)_([a-z][a-z0-9-]*)\.md$)"));
const QRegularExpression kCostRe(QStringLiteral(R"(^- Cost \(USD\): ([0-9.]+))"),
                                  QRegularExpression::MultilineOption);
const QRegularExpression kJenkinsRe(
    QStringLiteral("## Jenkins message that triggered this run\\n```\\n(.*?)\\n```"),
    QRegularExpression::DotMatchesEverythingOption);
// Only present when _write_cost_report was called with a known recipe:task
// signature (see yocto_triage_listener.py) - absent for infra-only
// failures, thread follow-ups, and catalog-write reports. The dashboard
// falls back to the call's own status (success/timeout/error/crash) when
// this is empty, rather than assuming a fix_status.
const QRegularExpression kSignatureRe(QStringLiteral(R"(^- Signature: `([0-9a-f]+)`)"),
                                        QRegularExpression::MultilineOption);

const int kMaxFeedEntries = 100;
}

TriageFeedSource::TriageFeedSource(const QString &reportsDir, QObject *parent)
    : QObject(parent), m_reportsDir(reportsDir)
{
    rescan();

    if (QDir(m_reportsDir).exists())
        m_watcher.addPath(m_reportsDir);
    else
        qWarning() << "Pasta de relatórios de custo não existe:" << m_reportsDir;

    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, &TriageFeedSource::rescan);
}

QVariantList TriageFeedSource::feed() const
{
    return m_feed;
}

void TriageFeedSource::rescan()
{
    QDir dir(m_reportsDir);
    if (!dir.exists()) {
        qWarning() << "Pasta de relatórios de custo não existe:" << m_reportsDir;
        return;
    }

    QVariantList entries;
    const QStringList files = dir.entryList(QStringList() << "*.md", QDir::Files, QDir::Name);
    for (const QString &fileName : files) {
        const QRegularExpressionMatch nameMatch = kFilenameRe.match(fileName);
        if (!nameMatch.hasMatch())
            continue;

        const QString date = nameMatch.captured(1);
        const QString time = nameMatch.captured(2);
        const QString id = nameMatch.captured(3);
        // "deterministic-<known-failures-entry-id>" - split the entry id
        // out so the dashboard can show which catalog entry matched, and
        // treat the status itself as the plain "deterministic" case below.
        QString status = nameMatch.captured(4);
        QString matchedEntry;
        static const QString kDeterministicPrefix = QStringLiteral("deterministic-");
        if (status.startsWith(kDeterministicPrefix)) {
            matchedEntry = status.mid(kDeterministicPrefix.length());
            status = QStringLiteral("deterministic");
        }

        QFile file(dir.filePath(fileName));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;
        const QString content = QString::fromUtf8(file.readAll());

        double cost = 0.0;
        const QRegularExpressionMatch costMatch = kCostRe.match(content);
        if (costMatch.hasMatch())
            cost = costMatch.captured(1).toDouble();

        QString jenkinsText;
        const QRegularExpressionMatch jenkinsMatch = kJenkinsRe.match(content);
        if (jenkinsMatch.hasMatch())
            jenkinsText = jenkinsMatch.captured(1).trimmed();

        // Empty when the report has no Signature: line - the dashboard must
        // treat that as "no known fix_status", not as any particular status.
        QString signature;
        const QRegularExpressionMatch signatureMatch = kSignatureRe.match(content);
        if (signatureMatch.hasMatch())
            signature = signatureMatch.captured(1);

        QVariantMap entry;
        entry[QStringLiteral("date")] =
            QStringLiteral("%1-%2-%3").arg(date.left(4), date.mid(4, 2), date.mid(6, 2));
        entry[QStringLiteral("time")] =
            QStringLiteral("%1:%2:%3").arg(time.left(2), time.mid(2, 2), time.mid(4, 2));
        entry[QStringLiteral("id")] = id;
        entry[QStringLiteral("status")] = status;
        entry[QStringLiteral("cost_usd")] = cost;
        entry[QStringLiteral("jenkins_text")] = jenkinsText;
        entry[QStringLiteral("signature")] = signature;
        entry[QStringLiteral("matched_entry")] = matchedEntry;
        entries.append(entry);
    }

    // O nome do arquivo já ordena por data/hora; só precisamos inverter pra
    // deixar o mais recente primeiro, e cortar num limite razoável.
    std::reverse(entries.begin(), entries.end());
    if (entries.size() > kMaxFeedEntries)
        entries = entries.mid(0, kMaxFeedEntries);

    if (entries != m_feed) {
        m_feed = entries;
        emit feedChanged();
    }
}
