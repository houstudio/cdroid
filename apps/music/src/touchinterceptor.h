/*********************************************************************************
 * Facade of com.android.music.TouchInterceptor — the ListView the original
 * app uses for every browsing list and for drag-to-reorder playlists.
 *
 * CDROID's media_picker_activity.xml keeps the
 * <com.android.music.TouchInterceptor> tag, so this subclass exists to make
 * the layout inflate; the drag-and-drop reordering (grabber + hover row) is
 * NOT ported — the view behaves as a plain ListView. TODO: port the
 * onInterceptTouchEvent/drag visuals if reorder is ever needed.
 *********************************************************************************/
#ifndef CDROID_MUSIC_TOUCHINTERCEPTOR_H
#define CDROID_MUSIC_TOUCHINTERCEPTOR_H

#include <widget/listview.h>

namespace cdroid {
namespace music {

class TouchInterceptor : public ListView {
public:
    using ListView::ListView;
};

} // namespace music
} // namespace cdroid

#endif // CDROID_MUSIC_TOUCHINTERCEPTOR_H
