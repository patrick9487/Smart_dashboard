#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>

class AppConfig : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString homePage READ homePage NOTIFY configLoaded)
    Q_PROPERTY(QJsonArray widgets READ widgets NOTIFY configLoaded)
    Q_PROPERTY(bool isLoaded READ isLoaded NOTIFY configLoaded)

public:
    explicit AppConfig(QObject *parent = nullptr);
    
    bool loadFromFile(const QString &filePath);
    QString homePage() const { return m_homePage; }
    QJsonArray widgets() const { return m_widgets; }
    bool isLoaded() const { return m_loaded; }

signals:
    void configLoaded();

private:
    QString m_homePage;
    QJsonArray m_widgets;
    bool m_loaded;
};

#endif // APPCONFIG_H


