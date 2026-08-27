# androidx.preference port (androidx.preference → cdroid). Stage coverage:
# data model (Preference/Group/Screen/Category), manager + inflater, two-state
# family, RecyclerView UI layer (ViewHolder/GroupAdapter/Fragment), dialog
# family (EditText/List/MultiSelect) and ExpandButton. Deferred: DiffUtil
# animations, PreferenceHeaderFragmentCompat two-pane, SeekBar/DropDown.

set(PREFERENCE_SOURCES
    preference/preference.cc
    preference/preferencedatastore.cc
    preference/preferencegroup.cc
    preference/preferencecategory.cc
    preference/preferencescreen.cc
    preference/preferencemanager.cc
    preference/preferenceinflater.cc
    preference/twostatepreference.cc
    preference/checkboxpreference.cc
    preference/switchpreference.cc
    preference/expandbutton.cc
    preference/preferenceviewholder.cc
    preference/preferencegroupadapter.cc
    preference/preferencefragment.cc
    preference/dialogpreference.cc
    preference/edittextpreference.cc
    preference/listpreference.cc
    preference/multiselectlistpreference.cc
    preference/preferencedialogfragment.cc
    preference/edittextpreferencedialogfragment.cc
    preference/listpreferencedialogfragment.cc
    preference/multiselectlistpreferencedialogfragment.cc
    preference/seekbarpreference.cc
    preference/dropdownpreference.cc
    preference/internal/preferenceimageview.cc
)
