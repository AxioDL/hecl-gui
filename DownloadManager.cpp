#include "DownloadManager.hpp"
#include "Common.hpp"

static const char AxioDLPublicKeyPEM[] =
"-----BEGIN PUBLIC KEY-----\n"
"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAvtshImzoP1a++9P5RK0k\n"
"btTOpwie7O7S/wWFZxwwbMezEPhjw2uu86TPqJe3P/1v+6xRKrEf9zFn/sKNygvD\n"
"bO64ZkJre4M46IYd0XxwIhiu7PBR+13CD+fqbrbYwPkoG090CP4MtZZN6mt5NAKB\n"
"QHoIj0wV5K/jJE9cBQxViwOqrxK05Cl/ivy0gRtpL7Ot6S+QHL3++rb6U2hWydIQ\n"
"kS+ucufKCIL77RcDwAc9vwNvzxf9EUU2pmq+EsEtLgRw3fR6BInoltOI8P9X5Wo6\n"
"/skeg92xZA++vv0neq5gjjDfa2A1zDgJRysz3Xps/AMlLOe55XCzXse9BpvChT+Z\n"
"pwIDAQAB\n"
"-----END PUBLIC KEY-----\n";

static const QSslKey AxioDLPublicKey =
    QSslKey({AxioDLPublicKeyPEM}, QSsl::Rsa, QSsl::Pem, QSsl::PublicKey);

void DownloadManager::_validateCert(QNetworkReply* reply)
{
    QSslCertificate peerCert = reply->sslConfiguration().peerCertificate();
    if (peerCert.publicKey() != AxioDLPublicKey)
    {
        auto cn = peerCert.subjectInfo(QSslCertificate::CommonName);
        if (!cn.empty())
            setError(QNetworkReply::SslHandshakeFailedError,
                     QStringLiteral("Certificate pinning mismatch \"") + cn.first() + "\"");
        else
            setError(QNetworkReply::SslHandshakeFailedError,
                     QStringLiteral("Certificate pinning mismatch"));
        reply->abort();
    }
}

static const QString Domain = QStringLiteral("https://releases.axiodl.com/");
static const QString Index = QStringLiteral("index.txt");

void DownloadManager::fetchIndex()
{
    if (m_indexInProgress)
        return;

    resetError();

    QString url = Domain + CurPlatformString + '/' + Index;
    m_indexInProgress = m_netManager.get(QNetworkRequest(url));
    connect(m_indexInProgress, SIGNAL(finished()),
            this, SLOT(indexFinished()));
    connect(m_indexInProgress, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(indexError(QNetworkReply::NetworkError)));
    connect(m_indexInProgress, SIGNAL(encrypted()),
            this, SLOT(indexValidateCert()));
}

void DownloadManager::fetchBinary(const QString& str, const QString& outPath)
{
    if (m_binaryInProgress)
        return;

    resetError();
    m_outPath = outPath;

    QString url = Domain + CurPlatformString + '/' + str;
    m_binaryInProgress = m_netManager.get(QNetworkRequest(url));
    connect(m_binaryInProgress, SIGNAL(finished()),
            this, SLOT(binaryFinished()));
    connect(m_binaryInProgress, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(binaryError(QNetworkReply::NetworkError)));
    connect(m_binaryInProgress, SIGNAL(encrypted()),
            this, SLOT(binaryValidateCert()));
    connect(m_binaryInProgress, SIGNAL(downloadProgress(qint64, qint64)),
            this, SLOT(binaryDownloadProgress(qint64, qint64)));

    if (m_progBar)
    {
        m_progBar->setEnabled(true);
        m_progBar->setValue(0);
    }
}

void DownloadManager::indexFinished()
{
    if (m_hasError)
        return;

    QStringList files;

    while (true)
    {
        if (m_indexInProgress->atEnd())
            break;
        QByteArray line = m_indexInProgress->readLine();
        if (line.isEmpty())
            continue;
        files.push_back(QString::fromUtf8(line).trimmed());
    }

    if (m_indexCompletionHandler)
        m_indexCompletionHandler(files);

    m_indexInProgress->deleteLater();
    m_indexInProgress = nullptr;
}

void DownloadManager::indexError(QNetworkReply::NetworkError error)
{
    setError(error, m_indexInProgress->errorString());
    m_indexInProgress->deleteLater();
    m_indexInProgress = nullptr;
}

void DownloadManager::indexValidateCert()
{
    _validateCert(m_indexInProgress);
}

void DownloadManager::binaryFinished()
{
    if (m_hasError)
        return;

    if (m_progBar)
        m_progBar->setValue(100);

    QFile fp(m_outPath);
    if (!fp.open(QIODevice::WriteOnly))
    {
        setError(QNetworkReply::ContentAccessDenied, fp.errorString());
        m_binaryInProgress->deleteLater();
        m_binaryInProgress = nullptr;
        return;
    }
    fp.write(m_binaryInProgress->readAll());
    fp.close();

    if (m_completionHandler)
        m_completionHandler(m_outPath);

    m_binaryInProgress->deleteLater();
    m_binaryInProgress = nullptr;
}

void DownloadManager::binaryError(QNetworkReply::NetworkError error)
{
    setError(error, m_binaryInProgress->errorString());
    m_binaryInProgress->deleteLater();
    m_binaryInProgress = nullptr;

    if (m_progBar)
        m_progBar->setEnabled(false);
}

void DownloadManager::binaryValidateCert()
{
    _validateCert(m_binaryInProgress);
}

void DownloadManager::binaryDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (m_progBar)
    {
        if (bytesReceived == bytesTotal)
            m_progBar->setValue(100);
        else
            m_progBar->setValue(int(bytesReceived * 100 / bytesTotal));
    }
}
