#include "HttpServiceMethod.h"
#include "HttpService.h"

#include <QJsonDocument>
#include <QUrlQuery>
#include <QBuffer>

#define NUMBER_TO_STRING(n) QString::number(n)

HttpServiceMethod::HttpServiceMethod()
{

}

HttpServiceMethod::~HttpServiceMethod()
{
}

HttpServiceMethod::HttpServiceMethod(QNetworkAccessManager::Operation op, HttpService *jsonHttpClient) :
    m_op(op), m_httpService(jsonHttpClient)
{
}

HttpServiceMethod &HttpServiceMethod::url(const QString &url)
{
    m_networkRequest.setUrl(QUrl(url));
    return *this;
}

HttpServiceMethod &HttpServiceMethod::header(const QString &key, const QString &value)
{
    m_networkRequest.setRawHeader(QByteArray(key.toStdString().data()), QByteArray(value.toStdString().data()));
    return *this;
}

HttpServiceMethod &HttpServiceMethod::jsonBody(const QVariant &jsonBody)
{
    if (jsonBody.type() == QVariant::Map) {
        m_jsonBody = QJsonObject::fromVariantMap(jsonBody.toMap());
    }
    else if (jsonBody.typeName() ==  QMetaType::typeName(QMetaType::QJsonObject)) {
        m_jsonBody = jsonBody.toJsonObject();
    }

    return *this;
}

HttpServiceMethod &HttpServiceMethod::onResponse(const QObject *reseceiver, const char *slot)
{
    m_slotsMap.insert(NUMBER_TO_STRING(HttpServiceMethod::onResponseMethod), {{slot, reseceiver}});
    return *this;
}

HttpServiceMethod &HttpServiceMethod::onError(const QObject *receiver, const char *slot)
{
    m_slotsMap.insert(NUMBER_TO_STRING(HttpServiceMethod::onErrorMethod), {{slot, receiver}});
    return *this;
}

bool HttpServiceMethod::exec()
{
    QNetworkReply* reply = NULL;
    QBuffer* sendBuffer = NULL;
    QJsonObject sendJson = m_jsonBody;
    if (!sendJson.isEmpty()) {
        QByteArray sendByteArray = QJsonDocument(sendJson).toJson();
        sendBuffer = new QBuffer(reply);
        sendBuffer->setData(sendByteArray);
    }

    reply = m_httpService->createRequest(m_op, m_networkRequest, sendBuffer);

    if (reply == NULL && sendBuffer != NULL) {
        sendBuffer->deleteLater();
        return false;
    }

    return new HttpRequest(reply, m_slotsMap);
}

HttpServiceMethod &HttpServiceMethod::queryParam(const QString &key, const QString &value)
{
    QUrl url(m_networkRequest.url());
    QUrlQuery urlQuery(url);

    urlQuery.addQueryItem(key, value);
    url.setQuery(urlQuery);

    m_networkRequest.setUrl(url);
    return *this;
}
