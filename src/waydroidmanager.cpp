#include "waydroidmanager.h"
#include "waydroidwindowembedder.h"

QObject* WaydroidManager::createWindowEmbedder(const QString &pkg) {
    auto *embedder = new WaydroidWindowEmbedder(this);
    embedder->setPackageName(pkg);
    return embedder;
}

