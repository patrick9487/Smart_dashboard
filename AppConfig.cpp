#include "AppConfig.h"
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QDir>

AppConfig::AppConfig(QObject *parent)
    : QObject(parent), m_loaded(false)
{
}

bool AppConfig::loadFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.exists()) {
        qWarning() << "Config file does not exist:" << filePath;
        // 列出可用的資源文件（用於調試）
        if (filePath.startsWith(":/")) {
            qDebug() << "Checking resource system...";
            QFileInfo info(filePath);
            qDebug() << "Resource path:" << filePath;
        }
        return false;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open config file:" << filePath << "Error:" << file.errorString();
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << error.errorString();
        return false;
    }

    if (!doc.isObject()) {
        qWarning() << "Config file is not a valid JSON object";
        return false;
    }

    QJsonObject obj = doc.object();
    m_homePage = obj.value("home_page").toString();
    m_widgets = obj.value("widgets").toArray();
    
    m_loaded = true;
    emit configLoaded();
    
    qDebug() << "Config loaded. Home page:" << m_homePage;
    qDebug() << "Widgets count:" << m_widgets.size();
    
    return true;
}

