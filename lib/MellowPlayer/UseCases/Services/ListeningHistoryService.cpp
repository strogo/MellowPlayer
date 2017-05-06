#include <QSet>
#include <MellowPlayer/UseCases/Logging/LoggingManager.hpp>
#include <MellowPlayer/UseCases/Player/IPlayer.hpp>
#include <MellowPlayer/UseCases/Interfaces/IListeningHistoryDataProvider.hpp>
#include "ListeningHistoryService.hpp"

USE_MELLOWPLAYER_NAMESPACE(Entities)
USE_MELLOWPLAYER_NAMESPACE(UseCases)

ListeningHistoryService::ListeningHistoryService(IListeningHistoryDataProvider& model, IPlayer& player,
                                                 IWorkDispatcher& workDispatcher):
        logger(LoggingManager::instance().getLogger("ListeningHistoryService")), dataProvider(model), player(player),
        workDispatcher(workDispatcher) {
    connect(&player, &IPlayer::currentSongChanged, this, &ListeningHistoryService::onCurrentSongChanged);
    connect(&player, &IPlayer::playbackStatusChanged, this, &ListeningHistoryService::onPlaybackStatusChanged);
}

void ListeningHistoryService::onPlaybackStatusChanged() {
    onCurrentSongChanged(player.getCurrentSong());
}

void ListeningHistoryService::onCurrentSongChanged(Song* song) {
    auto newEntry = ListeningHistoryEntry::fromData(song, player.getServiceName());
    workDispatcher.invoke([=]() mutable {
        auto previousEntry = previousEntryPerPlayer[player.getServiceName()];

        if (previousEntry.equals(newEntry) || !newEntry.isValid() ||
            player.getPlaybackStatus() != PlaybackStatus::Playing)
            return;

        newEntry.id = dataProvider.add(newEntry);
        entries.append(newEntry);
        emit entryAdded(newEntry);
        previousEntryPerPlayer[player.getServiceName()] = newEntry;
        LOG_DEBUG(logger, "new entry: " + song->toString() + ", id=" + QString("%1").arg(newEntry.id));
    });
}

const QList<ListeningHistoryEntry>& ListeningHistoryService::getEntries() const {
    return entries;
}

void ListeningHistoryService::clear() {
    workDispatcher.invoke([=]() mutable {
        dataProvider.clear();
        entries.clear();
        emit entriesCleared();
    });
}

void ListeningHistoryService::removeById(int entryId) {
    workDispatcher.invoke([=]() mutable {
        dataProvider.remove("id", QString("%1").arg(entryId));
        auto entry = entries.toSet().subtract(dataProvider.getAll().toSet()).toList().first();
        int index = entries.indexOf(entry);
        entries.removeAt(index);
        emit entryRemoved(index);
    });
}

void ListeningHistoryService::removeByService(const QString& serviceName) {
    workDispatcher.invoke([=]() mutable {
        dataProvider.remove("serviceName", serviceName);

        for (auto entry: entries.toSet().subtract(dataProvider.getAll().toSet()).toList()) {
            int index = entries.indexOf(entry);
            entries.removeAt(index);
            emit entryRemoved(index);
        }
    });
}

void ListeningHistoryService::initialize() {
    dataProvider.initialize();
    entries = this->dataProvider.getAll();
}