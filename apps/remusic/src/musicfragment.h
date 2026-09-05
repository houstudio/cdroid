// Port of com.wm.remusic.fragment.MusicFragment — the 单曲 tab: all local
// songs with the "播放全部" header row. SideBar A-Z fast scroll + sort menu
// land with the polish pass.
#ifndef __REMUSIC_MUSICFRAGMENT_H__
#define __REMUSIC_MUSICFRAGMENT_H__

#include <cdroid.h>
#include <fragment/fragment.h>

#include "musicinfo.h"

namespace remusic {

class MusicFragment : public cdroid::Fragment {
public:
    cdroid::View* onCreateView(cdroid::LayoutInflater* inflater, cdroid::ViewGroup* container,
                               cdroid::Bundle* savedInstanceState) override;
};

} // namespace remusic
#endif
