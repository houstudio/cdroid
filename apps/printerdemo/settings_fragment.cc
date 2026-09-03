/*********************************************************************************
 * SettingsFragment — brightness seekbar and the resource-driven language
 * picker: AssetManager.getNonSystemLocales() enumerates the app pak's locales,
 * PopupMenu offers them by self-name, applyLocale() switches and recreates.
 *********************************************************************************/
#include <core/app.h>
#include <cdroid.h>
#include <algorithm>
#include <vector>
#include <fragment/fragment.h>
#include <fragment/fragmentfactory.h>
#include <transition/slide.h>
#include <widget/textview.h>
#include <widget/seekbar.h>
#include <menu/menu.h>
#include <menu/menuitem.h>
#include <menu/popupmenu.h>
#include <content/assetmanager.h>
#include "printer_common.h"
#include "R.h"

// ---------------------------------------------------------------------------
class SettingsFragment : public cdroid::Fragment{
public:
    void onCreate(cdroid::Bundle* savedInstanceState) override{
        cdroid::Fragment::onCreate(savedInstanceState);
        setEnterTransition(new cdroid::Slide(cdroid::Gravity::END));
        setExitTransition(new cdroid::Slide(cdroid::Gravity::END));
    }
    cdroid::View* onCreateView(cdroid::LayoutInflater* inflater, cdroid::ViewGroup* container,
                               cdroid::Bundle*) override{
        return inflater->inflate(printerdemo::R::layout::fragment_settings, container, false);
    }
    void onViewCreated(cdroid::View* view, cdroid::Bundle*) override{
        cdroid::Fragment::onViewCreated(view, nullptr);
        cdroid::SeekBar* seek = (cdroid::SeekBar*)view->findViewById(printerdemo::R::id::seek_brightness);
        cdroid::TextView* tv = (cdroid::TextView*)view->findViewById(printerdemo::R::id::tv_brightness);
        if(seek && tv){
            cdroid::SeekBar::OnSeekBarChangeListener l;
            l.onProgressChanged = [tv](cdroid::SeekBar&, int progress, bool){
                tv->setText(std::to_string(progress) + "%");
            };
            l.onStartTrackingTouch = [](cdroid::SeekBar&){};
            l.onStopTrackingTouch  = [](cdroid::SeekBar&){};
            seek->setOnSeekBarChangeListener(l);
        }
        // Language picker: the offered set is what the app's resources actually
        // carry — AssetManager.getNonSystemLocales() enumerates the app pak's
        // values-<locale> tables; the unqualified values/ base (stored in the
        // arsc without a locale tag) is the app's base language, en-US here.
        if(cdroid::View* row = view->findViewById(printerdemo::R::id::row_language)){
            row->setOnClickListener([](cdroid::View& v){
                std::vector<std::string> tags{ "en-US" };   // the values/ base language
                for(const std::string& t : cdroid::App::getInstance().getAssets().getNonSystemLocales())
                    if(!t.empty() && std::find(tags.begin(), tags.end(), t) == tags.end())
                        tags.push_back(t);
                // Fire-and-forget (the unified transient-popup contract): the
                // menu owns itself after show() and self-destructs once its
                // dismiss cascade completes — no member, no delete, one fresh
                // menu per click. Gravity.RIGHT aligns the popup's right edge
                // with the row's right edge (the only horizontal alignment
                // PopupWindow special-cases, same as AOSP).
                cdroid::PopupMenu* menu = new cdroid::PopupMenu(v.getContext(), &v, cdroid::Gravity::RIGHT);
                cdroid::Menu* m = menu->getMenu();
                for(size_t i = 0; i < tags.size(); i++){
                    const cdroid::Locale l = cdroid::Locale::forLanguageTag(tags[i]);
                    cdroid::MenuItem* mi = m->add(cdroid::Menu::NONE, (int)i, (int)i,
                            l.getDisplayName(l));   // self-name, the picker convention
                    mi->setCheckable(true);
                    mi->setChecked(tags[i] == sLocaleTag);
                }
                menu->setOnMenuItemClickListener([tags](cdroid::MenuItem& item){
                    applyLocale(tags[item.getItemId()]);
                    return true;
                });
                menu->show();
            });
        }
    }
};
REGISTER_FRAGMENT(SettingsFragment);
