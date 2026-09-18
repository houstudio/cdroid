// Port of com.wm.remusic.fragment.MainFragment — the local-music home page
// (本地音乐/最近播放/下载管理/我的歌手 + playlist sections). Playlist CRUD is
// phase 3; the sections render as headers with the local set empty.
#ifndef __REMUSIC_MAINFRAGMENT_H__
#define __REMUSIC_MAINFRAGMENT_H__

#include <cdroid.h>
#include <fragment/fragment.h>
#include <widgetEx/recyclerview/recyclerview.h>

namespace remusic {

class MainFragment : public cdroid::Fragment {
public:
    cdroid::View* onCreateView(cdroid::LayoutInflater* inflater, cdroid::ViewGroup* container,
                               cdroid::Bundle* savedInstanceState) override;
    void reloadAdapter();
    void onResume() override;   // playlist CRUD on a child screen refreshes here
private:
    cdroid::RecyclerView::Adapter* mAdapter = nullptr;
};

} // namespace remusic
#endif
