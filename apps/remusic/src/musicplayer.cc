#include "musicplayer.h"

#include <core/context.h>

namespace remusic {

void MusicPlayer::init(cdroid::Context& context) {
    MediaPlaybackService::init(context);
    MediaPlaybackService::getInstance().restoreQueueIfNeeded();
}

} // namespace remusic
