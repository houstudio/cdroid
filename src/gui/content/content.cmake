# java.text date formatting: DateFormat / SimpleDateFormat / DateFormatSymbols,
# locale data served by the vendored i18n engine (internal to the library).
set(CONTENT_SOURCES
    # android.content.res: the resource stack (moved from core/)
    content/Locale.cc
    content/LocaleList.cc
    content/i18nbridge.cc
    content/asset.cc
    content/assetdir.cc
    content/assetmanager.cc
    content/resources.cc
    content/resourcesimpl.cc
    content/typedarray.cc
    content/typedvalue.cc
    content/xmlblock.cc
    content/numberformat.cc   # java.text.NumberFormat face over the i18n engine
    content/dateutils.cc     # android.text.format.DateUtils (in-tree subset)
    content/dateformat.cc
    content/dateformatsymbols.cc
    content/simpledateformat.cc
)
