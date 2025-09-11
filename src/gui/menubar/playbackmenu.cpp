/*
 * Fooyin
 * Copyright © 2023, Luke Taylor <LukeT1@proton.me>
 *
 * Fooyin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Fooyin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Fooyin.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "playbackmenu.h"

#include <core/coresettings.h>
#include <core/player/playercontroller.h>
#include <gui/guiconstants.h>
#include <gui/guisettings.h>
#include <utils/actions/actioncontainer.h>
#include <utils/actions/actionmanager.h>
#include <utils/actions/command.h>
#include <utils/settings/settingsmanager.h>
#include <utils/utils.h>

#include <QAction>
#include <QActionGroup>
#include <QMenu>

namespace Fooyin {
PlaybackMenu::PlaybackMenu(ActionManager* actionManager, PlayerController* playerController, SettingsManager* settings,
                           QObject* parent)
    : QObject{parent}
    , m_actionManager{actionManager}
    , m_playerController{playerController}
    , m_settings{settings}
    , m_playIcon{Utils::iconFromTheme(Constants::Icons::Play)}
    , m_pauseIcon{Utils::iconFromTheme(Constants::Icons::Pause)}
    , m_stop{new QAction(Utils::iconFromTheme(Constants::Icons::Stop), tr("&Stop"), this)}
    , m_playPause{new QAction(m_playIcon, tr("&Play"), this)}
    , m_previous{new QAction(Utils::iconFromTheme(Constants::Icons::Prev), tr("P&revious"), this)}
    , m_next{new QAction(Utils::iconFromTheme(Constants::Icons::Next), tr("&Next"), this)}
    , m_defaultPlayback{new QAction(tr("&Default"), this)}
    , m_repeatTrack{new QAction(tr("Repeat &track"), this)}
    , m_repeatAlbum{new QAction(tr("Repeat &album"), this)}
    , m_repeatPlaylist{new QAction(tr("Repeat &playlist"), this)}
    , m_shuffleTracks{new QAction(tr("&Shuffle tracks"), this)}
    , m_shuffleAlbums{new QAction(tr("Shu&ffle albums"), this)}
    , m_random{new QAction(tr("&Random"), this)}
    , m_stopAfterCurrent{new QAction(tr("Stop &after current"), this)}
    , m_resetStopAfterCurrent{new QAction(tr("&Reset the above after stopping"), this)}
{
    auto* playbackMenu = m_actionManager->actionContainer(Constants::Menus::Playback);

    const QStringList playbackCategory = {tr("Playback")};

    QObject::connect(m_playerController, &PlayerController::playStateChanged, this,
                     [this](Player::PlayState state) { updatePlayPause(state); });
    QObject::connect(m_playerController, &PlayerController::playModeChanged, this,
                     [this](Playlist::PlayModes mode) { updatePlayMode(mode); });

    m_stop->setStatusTip(tr("Stop playback"));
    auto* stopCmd = actionManager->registerAction(m_stop, Constants::Actions::Stop);
    stopCmd->setCategories(playbackCategory);
    stopCmd->setAttribute(ProxyAction::UpdateText);
    playbackMenu->addAction(stopCmd);

    m_playPause->setStatusTip(tr("Pause or unpause playback"));
    auto* playPauseCmd = actionManager->registerAction(m_playPause, Constants::Actions::PlayPause);
    playPauseCmd->setCategories(playbackCategory);
    playPauseCmd->setDescription(tr("Play/Pause"));
    playPauseCmd->setDefaultShortcut(Qt::Key_Space);
    playPauseCmd->setAttribute(ProxyAction::UpdateText);
    playPauseCmd->setAttribute(ProxyAction::UpdateIcon);
    playbackMenu->addAction(playPauseCmd);

    m_next->setStatusTip(tr("Start playing the next track in the current playlist"));
    auto* nextCmd = actionManager->registerAction(m_next, Constants::Actions::Next);
    nextCmd->setCategories(playbackCategory);
    nextCmd->setAttribute(ProxyAction::UpdateText);
    playbackMenu->addAction(nextCmd);

    m_previous->setStatusTip(tr("Start playing the previous track in the current playlist"));
    auto* prevCmd = actionManager->registerAction(m_previous, Constants::Actions::Previous);
    prevCmd->setCategories(playbackCategory);
    prevCmd->setAttribute(ProxyAction::UpdateText);
    playbackMenu->addAction(prevCmd);

    QObject::connect(m_stop, &QAction::triggered, playerController, &PlayerController::stop);
    QObject::connect(m_playPause, &QAction::triggered, playerController, &PlayerController::playPause);
    QObject::connect(m_next, &QAction::triggered, playerController, &PlayerController::next);
    QObject::connect(m_previous, &QAction::triggered, playerController, &PlayerController::previous);

    playbackMenu->addSeparator();

    QStringList orderCategory{playbackCategory};
    orderCategory.append(tr("Order"));

    auto* orderMenu = m_actionManager->createMenu(Constants::Menus::PlaybackOrder);
    orderMenu->menu()->setTitle(tr("&Order"));
    playbackMenu->addMenu(orderMenu);

    m_defaultPlayback->setCheckable(true);
    m_repeatTrack->setCheckable(true);
    m_repeatAlbum->setCheckable(true);
    m_repeatPlaylist->setCheckable(true);
    m_shuffleTracks->setCheckable(true);
    m_shuffleAlbums->setCheckable(true);
    m_random->setCheckable(true);

    m_defaultPlayback->setStatusTip(tr("Set playback order to default"));
    auto* defaultCmd = actionManager->registerAction(m_defaultPlayback, Constants::Actions::PlaybackDefault);
    defaultCmd->setCategories(orderCategory);
    defaultCmd->setAttribute(ProxyAction::UpdateText);
    orderMenu->addAction(defaultCmd);

    m_repeatTrack->setStatusTip(tr("Set playback order to shuffle tracks in the current playlist"));
    auto* repeatTrackCmd = actionManager->registerAction(m_repeatTrack, Constants::Actions::RepeatTrack);
    repeatTrackCmd->setCategories(orderCategory);
    repeatTrackCmd->setAttribute(ProxyAction::UpdateText);
    orderMenu->addAction(repeatTrackCmd);

    m_repeatAlbum->setStatusTip(tr("Set playback order to repeat the current album"));
    auto* repeatAlbumCmd = actionManager->registerAction(m_repeatAlbum, Constants::Actions::RepeatAlbum);
    repeatAlbumCmd->setCategories(orderCategory);
    repeatAlbumCmd->setAttribute(ProxyAction::UpdateText);
    orderMenu->addAction(repeatAlbumCmd);

    m_repeatPlaylist->setStatusTip(tr("Set playback order to repeat the current playlist"));
    auto* repeatPlaylistCmd = actionManager->registerAction(m_repeatPlaylist, Constants::Actions::RepeatPlaylist);
    repeatPlaylistCmd->setCategories(orderCategory);
    repeatPlaylistCmd->setAttribute(ProxyAction::UpdateText);
    orderMenu->addAction(repeatPlaylistCmd);

    m_shuffleAlbums->setStatusTip(tr("Set playback order to shuffle albums in the current playlist"));
    auto* shuffleAlbumsCmd = actionManager->registerAction(m_shuffleAlbums, Constants::Actions::ShuffleAlbums);
    shuffleAlbumsCmd->setCategories(orderCategory);
    shuffleAlbumsCmd->setAttribute(ProxyAction::UpdateText);
    orderMenu->addAction(shuffleAlbumsCmd);

    m_shuffleTracks->setStatusTip(tr("Set playback order to shuffle tracks in the current playlist"));
    auto* shuffleCmd = actionManager->registerAction(m_shuffleTracks, Constants::Actions::ShuffleTracks);
    shuffleCmd->setCategories(orderCategory);
    shuffleCmd->setAttribute(ProxyAction::UpdateText);
    orderMenu->addAction(shuffleCmd);

    m_random->setStatusTip(tr("Set playback order to play a random track in the current playlist"));
    auto* randomCmd = actionManager->registerAction(m_random, Constants::Actions::Random);
    randomCmd->setCategories(orderCategory);
    randomCmd->setAttribute(ProxyAction::UpdateText);
    orderMenu->addAction(randomCmd);

    QObject::connect(m_defaultPlayback, &QAction::triggered, this, [this]() { setPlayMode(Playlist::Default); });
    QObject::connect(m_repeatTrack, &QAction::triggered, this,
                     [this]() { setPlayMode(Playlist::PlayMode::RepeatTrack); });
    QObject::connect(m_repeatAlbum, &QAction::triggered, this,
                     [this]() { setPlayMode(Playlist::PlayMode::RepeatAlbum); });
    QObject::connect(m_repeatPlaylist, &QAction::triggered, this,
                     [this]() { setPlayMode(Playlist::PlayMode::RepeatPlaylist); });
    QObject::connect(m_shuffleTracks, &QAction::triggered, this,
                     [this]() { setPlayMode(Playlist::PlayMode::ShuffleTracks); });
    QObject::connect(m_shuffleAlbums, &QAction::triggered, this,
                     [this]() { setPlayMode(Playlist::PlayMode::ShuffleAlbums); });
    QObject::connect(m_random, &QAction::triggered, this, [this]() { setPlayMode(Playlist::PlayMode::Random); });

    auto* followPlayback = new QAction(tr("Cursor follows play&back"), this);
    followPlayback->setStatusTip(tr("Select the currently playing track when changed"));
    auto* followCursor = new QAction(tr("Playback follows &cursor"), this);
    followCursor->setStatusTip(tr("Start playback of the currently selected track on next"));

    m_stopAfterCurrent->setStatusTip(tr("Stop playback at the end of the current track"));
    auto* stopCurrentCmd = actionManager->registerAction(m_stopAfterCurrent, Constants::Actions::StopAfterCurrent);
    stopCurrentCmd->setCategories(playbackCategory);
    stopCurrentCmd->setAttribute(ProxyAction::UpdateText);

    m_resetStopAfterCurrent->setStatusTip(tr("Reset 'Stop after current' after stopping"));
    auto* resetStopCurrentCmd
        = actionManager->registerAction(m_resetStopAfterCurrent, Constants::Actions::StopAfterCurrentReset);
    resetStopCurrentCmd->setCategories(playbackCategory);
    resetStopCurrentCmd->setAttribute(ProxyAction::UpdateText);

    m_stopAfterCurrent->setCheckable(true);
    m_resetStopAfterCurrent->setCheckable(true);
    followPlayback->setCheckable(true);
    followCursor->setCheckable(true);

    m_stopAfterCurrent->setChecked(m_settings->value<Settings::Core::StopAfterCurrent>());
    m_resetStopAfterCurrent->setChecked(m_settings->value<Settings::Core::ResetStopAfterCurrent>());
    followPlayback->setChecked(m_settings->value<Settings::Gui::CursorFollowsPlayback>());
    followCursor->setChecked(m_settings->value<Settings::Gui::PlaybackFollowsCursor>());

    QObject::connect(m_stopAfterCurrent, &QAction::triggered, this,
                     [this](bool enabled) { m_settings->set<Settings::Core::StopAfterCurrent>(enabled); });
    QObject::connect(m_resetStopAfterCurrent, &QAction::triggered, this,
                     [this](bool enabled) { m_settings->set<Settings::Core::ResetStopAfterCurrent>(enabled); });
    QObject::connect(followPlayback, &QAction::triggered, this,
                     [this](bool enabled) { m_settings->set<Settings::Gui::CursorFollowsPlayback>(enabled); });
    QObject::connect(followCursor, &QAction::triggered, this,
                     [this](bool enabled) { m_settings->set<Settings::Gui::PlaybackFollowsCursor>(enabled); });

    m_settings->subscribe<Settings::Core::StopAfterCurrent>(
        this, [this](bool enabled) { m_stopAfterCurrent->setChecked(enabled); });
    m_settings->subscribe<Settings::Core::ResetStopAfterCurrent>(
        this, [this](bool enabled) { m_resetStopAfterCurrent->setChecked(enabled); });
    m_settings->subscribe<Settings::Gui::CursorFollowsPlayback>(
        this, [followPlayback](bool enabled) { followPlayback->setChecked(enabled); });
    m_settings->subscribe<Settings::Gui::PlaybackFollowsCursor>(
        this, [followCursor](bool enabled) { followCursor->setChecked(enabled); });

    playbackMenu->addAction(followPlayback);
    playbackMenu->addAction(followCursor);
    playbackMenu->addSeparator();
    playbackMenu->addAction(stopCurrentCmd->action());
    playbackMenu->addAction(resetStopCurrentCmd->action());

    updatePlayPause(m_playerController->playState());
    updatePlayMode(m_playerController->playMode());
}

void PlaybackMenu::updatePlayPause(Player::PlayState state) const
{
    if(state == Player::PlayState::Playing) {
        m_playPause->setText(tr("&Pause"));
        m_playPause->setIcon(m_pauseIcon);
    }
    else {
        m_playPause->setText(tr("&Play"));
        m_playPause->setIcon(m_playIcon);
    }
}

void PlaybackMenu::updatePlayMode(Playlist::PlayModes mode) const
{
    m_defaultPlayback->setChecked(
        mode & Playlist::Default
        || !(mode & (Playlist::RepeatTrack | Playlist::RepeatAlbum | Playlist::RepeatPlaylist)));
    m_repeatTrack->setChecked(mode & Playlist::RepeatTrack);
    m_repeatAlbum->setChecked(mode & Playlist::RepeatAlbum);
    m_repeatPlaylist->setChecked(mode & Playlist::RepeatPlaylist);
    m_shuffleTracks->setChecked(mode & Playlist::ShuffleTracks);
    m_shuffleAlbums->setChecked(mode & Playlist::ShuffleAlbums);
    m_random->setChecked(mode & Playlist::Random);
}

void PlaybackMenu::setPlayMode(Playlist::PlayMode mode) const
{
    Playlist::PlayModes currentMode = m_playerController->playMode();

    const bool isModeActive = currentMode & mode;

    if(isModeActive) {
        currentMode &= ~mode;
    }
    else {
        switch(mode) {
            case Playlist::Default:
                currentMode |= Playlist::Default;
                currentMode &= ~(Playlist::RepeatTrack | Playlist::RepeatAlbum | Playlist::RepeatPlaylist);
                break;
            case Playlist::RepeatTrack:
                currentMode |= Playlist::RepeatTrack;
                currentMode &= ~(Playlist::RepeatAlbum | Playlist::RepeatPlaylist);
                break;
            case Playlist::RepeatAlbum:
                currentMode |= Playlist::RepeatAlbum;
                currentMode &= ~(Playlist::RepeatTrack | Playlist::RepeatPlaylist);
                break;
            case Playlist::RepeatPlaylist:
                currentMode |= Playlist::RepeatPlaylist;
                currentMode &= ~(Playlist::RepeatTrack | Playlist::RepeatAlbum);
                break;
            case Playlist::ShuffleTracks:
                currentMode |= Playlist::ShuffleTracks;
                currentMode &= ~(Playlist::ShuffleAlbums | Playlist::Random);
                break;
            case Playlist::ShuffleAlbums:
                currentMode |= Playlist::ShuffleAlbums;
                currentMode &= ~(Playlist::ShuffleTracks | Playlist::Random);
                break;
            case Playlist::Random:
                currentMode |= Playlist::Random;
                currentMode &= ~(Playlist::ShuffleAlbums | Playlist::ShuffleTracks);
                break;
            default:
                currentMode |= mode;
                break;
        }
    }

    m_playerController->setPlayMode(currentMode);
    if(!isModeActive) {
        updatePlayMode(m_playerController->playMode());
    }
}
} // namespace Fooyin

#include "moc_playbackmenu.cpp"
