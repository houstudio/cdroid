#include <core/intent.h>
#include <core/uri.h>
#include <sstream>
namespace cdroid{

Intent::Intent() {
    mData=nullptr;
}

Intent::Intent(const Intent& o):Intent(o,COPY_MODE_ALL){
}

Intent::Intent(const Intent& o, int copyMode) {
    mAction = o.mAction;
    mData = o.mData;
    mType = o.mType;
    mPackage = o.mPackage;
    //mComponent = o.mComponent;

    mCategories = o.mCategories;

    /*if (copyMode != COPY_MODE_FILTER) {
        mFlags = o.mFlags;
        mContentUserHint = o.mContentUserHint;
        mLaunchToken = o.mLaunchToken;
        if (o.mSourceBounds != nullptr) {
            this.mSourceBounds = new Rect(o.mSourceBounds);
        }
        if (o.mSelector != nullptr) {
            this.mSelector = new Intent(o.mSelector);
        }

        if (copyMode != COPY_MODE_HISTORY) {
            if (o.mExtras != nullptr) {
                this.mExtras = new Bundle(o.mExtras);
            }
            if (o.mClipData != nullptr) {
                this.mClipData = new ClipData(o.mClipData);
            }
        } else {
            if (o.mExtras != null && !o.mExtras.maybeIsEmpty()) {
                this.mExtras = Bundle.STRIPPED;
            }
        }
    }*/
}


Intent* Intent::cloneFilter() {
    return new Intent(*this, COPY_MODE_FILTER);
}

Intent::Intent(const std::string& action) {
    mData = nullptr;
    setAction(action);
}

Intent::Intent(const std::string& action, Uri* uri) {
    setAction(action);
    mData = uri;
}
 
/*Intent::Intent(Context* packageContext, Class<?> cls) {
    mComponent = new ComponentName(packageContext, cls);
}

Intent::Intent(std::string action, Uri uri,Context* packageContext, Class<?> cls) {
    setAction(action);
    mData = uri;
    mComponent = new ComponentName(packageContext, cls);
}

Intent* Intent::makeMainActivity(ComponentName mainActivity) {
    Intent* intent = new Intent(ACTION_MAIN);
    intent->setComponent(mainActivity);
    intent->addCategory(CATEGORY_LAUNCHER);
    return intent;
}*/

Intent::~Intent(){
    delete mData;
}

Intent* Intent::makeMainSelectorActivity(const std::string& selectorAction,const std::string& selectorCategory) {
    Intent* intent = new Intent(ACTION_MAIN);
    intent->addCategory(CATEGORY_LAUNCHER);
    Intent* selector = new Intent();
    selector->setAction(selectorAction);
    selector->addCategory(selectorCategory);
    intent->setSelector(selector);
    return intent;
}

/*Intent* Intent::makeRestartActivityTask(ComponentName mainActivity) {
    Intent* intent = makeMainActivity(mainActivity);
    intent->addFlags(Intent::FLAG_ACTIVITY_NEW_TASK | Intent::FLAG_ACTIVITY_CLEAR_TASK);
    return intent;
}*/

Intent* Intent::parseUri(const std::string& uri,int flags) {
    int i = 0;
    bool androidApp = uri.compare(0,12,"android-app:");

    // Validate intent scheme if requested.
    if ((flags&(URI_INTENT_SCHEME|URI_ANDROID_APP_SCHEME)) != 0) {
        if (!uri.compare(0,7,"intent:") && !androidApp) {
            Intent* intent = new Intent(ACTION_VIEW);
            intent->setData(new Uri(uri));
            return intent;
        }
    }

    i = uri.find_last_of("#");
    // simple case
    if (i == std::string::npos) {
        if (!androidApp) {
            return new Intent(ACTION_VIEW, new Uri(uri));
        }

    // old format Intent URI
    } else if (uri.compare(i,8,"#Intent;")) {
        if (!androidApp) {
            return getIntentOld(uri, flags);
        } else {
            i = -1;
        }
    }

    // new format
    Intent* intent = new Intent(ACTION_VIEW);
    Intent* baseIntent = intent;
    bool explicitAction = false;
    bool inSelector = false;

    // fetch data part, if present
    std::string scheme;
    std::string data;
    if (i >= 0) {
        data = uri.substr(0, i);
        i += 8; // length of "#Intent;"
    } else {
        data = uri;
    }

    // loop over contents of Intent, all name=value;
    while (i >= 0 && uri.compare(i,3,"end")) {
        int eq = uri.find('=', i);
        if (eq < 0) eq = i-1;
        int semi = uri.find(';', i);
        std::string value = eq < semi ? Uri::decode(uri.substr(eq + 1, semi)) : "";

        // action
        if (uri.compare(i,7,"action=")==0) {
            intent->setAction(value);
            if (!inSelector) {
                explicitAction = true;
            }
        }
        // categories
        else if (uri.compare(i,9,"category=")==0) {
            intent->addCategory(value);
        }
        // type
        else if (uri.compare(i,5,"type=")==0) {
            intent->mType = value;
        }
        // launch flags
        else if (uri.compare(i,12,"launchFlags=")==0) {
            intent->mFlags = std::stoi(value);//Integer.decode(value).intValue();
            if ((flags& URI_ALLOW_UNSAFE) == 0) {
                intent->mFlags &= ~IMMUTABLE_FLAGS;
            }
        }
        // package
        else if (uri.compare(i,8,"package=")==0) {
            intent->mPackage = value;
        }
        // component
        else if (uri.compare(i,10,"component=")==0) {
            //intent->mComponent = ComponentName.unflattenFromString(value);
        }
        // scheme
        else if (uri.compare(i,7,"scheme=")==0) {
            if (inSelector) {
                //intent->mData = Uri::parse(value + ":");
            } else {
                scheme = value;
            }
        }
        // source bounds
        else if (uri.compare(i,13,"sourceBounds=")==0) {
            //intent.mSourceBounds = Rect.unflattenFromString(value);
        }
        // selector
        else if (semi == (i+3) && uri.compare(i,3,"SEL")==0) {
            intent = new Intent();
            inSelector = true;
        } else {//extra
            std::string key = "";//Uri::decode(uri.substr(i + 2, eq));
            // create Bundle if it doesn't already exist
            if (intent->mExtras == nullptr) intent->mExtras = new Bundle();
            Bundle* b = intent->mExtras;
            // add EXTRA
            if      (uri.compare(i,2,"S.")==0) b->putString(key, value);
            else if (uri.compare(i,2,"B.")==0) b->putBoolean(key, value.size()&&(value[0]=='t'||value[0]=='T'));//Boolean.parseBoolean(value));
            else if (uri.compare(i,2,"b.")==0) b->putByte(key, static_cast<char>(std::stoi(value)));//Byte.parseByte(value));
            //else if (uri.compare(i,2,"c.")==0) b->putChar(key, value.at(0));
            else if (uri.compare(i,2,"d.")==0) b->putDouble(key, std::stod(value));//Double.parseDouble(value));
            else if (uri.compare(i,2,"f.")==0) b->putFloat(key, std::stof(value));//Float.parseFloat(value));
            else if (uri.compare(i,2,"i.")==0) b->putInt(key, std::stoi(value));//Integer.parseInt(value));
            else if (uri.compare(i,2,"l.")==0) b->putLong(key, std::stol(value));//Long.parseLong(value));
            else if (uri.compare(i,2,"s.")==0) b->putShort(key, std::stoi(value));//Short.parseShort(value));
            else throw std::logic_error("unknown EXTRA type");//URISyntaxException(uri, "unknown EXTRA type", i);
        }

        // move to the next item
        i = semi + 1;
    }

    if (inSelector) {
        // The Intent had a selector; fix it up.
        if (baseIntent->mPackage.empty()) {
            baseIntent->setSelector(intent);
        }
        intent = baseIntent;
    }

    if (!data.empty()) {
        if (data.compare(0,7,"intent:")==0) {
            data = data.substr(0,7);
            if (!scheme.empty()) {
                data = scheme + ':' + data;
            }
        } else if (data.compare(0,12,"android-app:")==0) {
            if (data.at(12) == '/' && data.at(13) == '/') {
                // Correctly formed android-app, first part is package name.
                int end = data.find('/', 14);
                if (end < 0) {
                    // All we have is a package name.
                    intent->mPackage = data.substr(0,14);
                    if (!explicitAction) {
                        intent->setAction(ACTION_MAIN);
                    }
                    data = "";
                } else {
                    // Target the Intent at the given package name always.
                    std::string authority;
                    intent->mPackage = data.substr(14, end-14);
                    int newEnd;
                    if ((end+1) < data.length()) {
                        if ((newEnd=data.find('/', end+1)) !=std::string::npos) {
                            // Found a scheme, remember it.
                            scheme = data.substr(end+1, newEnd-end-1);
                            end = newEnd;
                            if (end < data.length() && (newEnd=data.find('/', end+1)) >= 0) {
                                // Found a authority, remember it.
                                authority = data.substr(end+1, newEnd-end-1);
                                end = newEnd;
                            }
                        } else {
                            // All we have is a scheme.
                            scheme = data.substr(end+1);
                        }
                    }
                    if (scheme.empty()) {
                        // If there was no scheme, then this just targets the package.
                        if (!explicitAction) {
                            intent->setAction(ACTION_MAIN);
                        }
                        data = "";
                    } else if (authority.empty()) {
                        data = scheme + ":";
                    } else {
                        data = scheme + "://" + authority + data.substr(end);
                    }
                }
            } else {
                data = "";
            }
        }

        if (data.length() > 0) {
            intent->mData = new Uri(data);
        }
    }

    return intent;
}

Intent* Intent::getIntentOld(const std::string& uri) {
    return getIntentOld(uri, 0);
}

Intent* Intent::getIntentOld(const std::string& uri, int flags) {
    Intent* intent;

    auto i = uri.find_last_of('#');
    if (i !=std::string::npos) {
        std::string action;
        int intentFragmentStart = i;
        bool isIntentFragment = false;

        i++;

        if (uri.compare(i,7, "action(")==0) {
            isIntentFragment = true;
            i += 7;
            int j = uri.find(')', i);
            action = uri.substr(i, j-i);
            i = j + 1;
        }

        intent = new Intent(action);

        if (uri.compare(i,11, "categories(")==0) {
            isIntentFragment = true;
            i += 11;
            int j = uri.find(')', i);
            while (i < j) {
                int sep = uri.find('!', i);
                if (sep < 0 || sep > j) sep = j;
                if (i < sep) {
                    intent->addCategory(uri.substr(i, sep-i));
                }
                i = sep + 1;
            }
            i = j + 1;
        }

        if (uri.compare(i,5,"type(")==0) {
            isIntentFragment = true;
            i += 5;
            int j = uri.find(')', i);
            intent->mType = uri.substr(i, j-i);
            i = j + 1;
        }

        if (uri.compare(i,12,"launchFlags(")==0) {
            isIntentFragment = true;
            i += 12;
            int j = uri.find(')', i);
            intent->mFlags = std::stoi(uri.substr(i, j-i));//Integer.decode(uri.substr(i, j-i)).intValue();
            if ((flags& URI_ALLOW_UNSAFE) == 0) {
                intent->mFlags &= ~IMMUTABLE_FLAGS;
            }
            i = j + 1;
        }

        if (uri.compare(i,10, "component(")==0) {
            isIntentFragment = true;
            i += 10;
            int j = uri.find(')', i);
            int sep = uri.find('!', i);
            if (sep >= 0 && sep < j) {
                std::string pkg = uri.substr(i, sep-i);
                std::string cls = uri.substr(sep + 1, j-sep-1);
                //intent->mComponent = new ComponentName(pkg, cls);
            }
            i = j + 1;
        }

        if (uri.compare(i,7,"extras(")==0) {
            isIntentFragment = true;
            i += 7;

            const int closeParen = uri.find(')', i);
            if (closeParen == -1) throw std::logic_error("EXTRA missing trailing ')'");//new URISyntaxException(uri,"EXTRA missing trailing ')'", i);

            while (i < closeParen) {
                // fetch the key value
                int j = uri.find('=', i);
                if (j <= i + 1 || i >= closeParen) {
                    throw std::logic_error("EXTRA missing '='");//new URISyntaxException(uri, "EXTRA missing '='", i);
                }
                char type = uri.at(i);
                i++;
                std::string key = uri.substr(i, j-i);
                i = j + 1;

                // get type-value
                j = uri.find('!', i);
                if (j == -1 || j >= closeParen) j = closeParen;
                if (i >= j) throw std::logic_error("EXTRA missing '!'");//new URISyntaxException(uri, "EXTRA missing '!'", i);
                std::string value = uri.substr(i, j-i);
                i = j;

                // create Bundle if it doesn't already exist
                if (intent->mExtras == nullptr) intent->mExtras = new Bundle();

                // add item to bundle
                try {
                    switch (type) {
                        case 'S':
                            intent->mExtras->putString(key, Uri::decode(value));
                            break;
                        case 'B':
                            intent->mExtras->putBoolean(key, value.size()&&(value[0]=='T'||value[0]=='t'));//Boolean.parseBoolean(value));
                            break;
                        case 'b':
                            intent->mExtras->putByte(key, static_cast<int8_t>(std::stoi(value)));//Byte.parseByte(value));
                            break;
                        case 'c':
                            //intent->mExtras->putChar(key, Uri::decode(value).at(0));
                            break;
                        case 'd':
                            intent->mExtras->putDouble(key, std::stod(value));//Double.parseDouble(value));
                            break;
                        case 'f':
                            intent->mExtras->putFloat(key, std::stof(value));//Float.parseFloat(value));
                            break;
                        case 'i':
                            intent->mExtras->putInt(key, std::stoi(value));//Integer.parseInt(value));
                            break;
                        case 'l':
                            intent->mExtras->putLong(key, std::stoll(value));//Long.parseLong(value));
                            break;
                        case 's':
                            intent->mExtras->putShort(key, static_cast<short>(std::stoi(value)));//Short.parseShort(value));
                            break;
                        default:
                            throw std::logic_error("EXTRA has unknown type");//new URISyntaxException(uri, "EXTRA has unknown type", i);
                    }
                } catch (std::exception& e) {
                    //throw new URISyntaxException(uri, "EXTRA value can't be parsed", i);
                }

                char ch = uri.at(i);
                if (ch == ')') break;
                if (ch != '!') throw std::logic_error("EXTRA missing '!'");//new URISyntaxException(uri, "EXTRA missing '!'", i);
                i++;
            }
        }

        if (isIntentFragment) {
            intent->mData = new Uri(uri.substr(0, intentFragmentStart));
        } else {
            intent->mData = new Uri(uri);
        }

        if (intent->mAction.empty()) {
            // By default, if no action is specified, then use VIEW.
            intent->mAction = ACTION_VIEW;
        }

    } else {
        intent = new Intent(ACTION_VIEW, new Uri(uri));
    }

    return intent;
}

#if 0
interface CommandOptionHandler {
    bool handleOption(std::string opt, ShellCommand cmd);
}

Intent* Intent::parseCommandArgs(ShellCommand cmd, CommandOptionHandler optionHandler){
    Intent intent = new Intent();
    Intent baseIntent = intent;
    bool hasIntentInfo = false;

    Uri data = null;
    std::string type = null;

    std::string opt;
    while ((opt=cmd.getNextOption()) != nullptr) {
        switch (opt) {
            case "-a":
                intent.setAction(cmd.getNextArgRequired());
                if (intent == baseIntent) {
                    hasIntentInfo = true;
                }
                break;
            case "-d":
                data = Uri.parse(cmd.getNextArgRequired());
                if (intent == baseIntent) {
                    hasIntentInfo = true;
                }
                break;
            case "-t":
                type = cmd.getNextArgRequired();
                if (intent == baseIntent) {
                    hasIntentInfo = true;
                }
                break;
            case "-c":
                intent.addCategory(cmd.getNextArgRequired());
                if (intent == baseIntent) {
                    hasIntentInfo = true;
                }
                break;
            case "-e":
            case "--es": {
                std::string key = cmd.getNextArgRequired();
                std::string value = cmd.getNextArgRequired();
                intent.putExtra(key, value);
            }
            break;
            case "--esn": {
                std::string key = cmd.getNextArgRequired();
                intent.putExtra(key, (std::string) nullptr);
            }
            break;
            case "--ei": {
                std::string key = cmd.getNextArgRequired();
                std::string value = cmd.getNextArgRequired();
                intent.putExtra(key, Integer.decode(value));
            }
            break;
            case "--eu": {
                std::string key = cmd.getNextArgRequired();
                std::string value = cmd.getNextArgRequired();
                intent.putExtra(key, Uri.parse(value));
            }
            break;
            case "--ecn": {
                std::string key = cmd.getNextArgRequired();
                std::string value = cmd.getNextArgRequired();
                ComponentName cn = ComponentName.unflattenFromString(value);
                if (cn == nullptr)
                    throw new IllegalArgumentException("Bad component name: " + value);
                intent.putExtra(key, cn);
            }
            break;
            case "--eia": {
                std::string key = cmd.getNextArgRequired();
                std::string value = cmd.getNextArgRequired();
                std::string[] strings = value.split(",");
                int[] list = new int[strings.length];
                for (int i = 0; i < strings.length; i++) {
                    list[i] = Integer.decode(strings[i]);
                }
                intent.putExtra(key, list);
            }
            break;
            case "--eial": {
                std::string key = cmd.getNextArgRequired();
                std::string value = cmd.getNextArgRequired();
                std::string[] strings = value.split(",");
                ArrayList<Integer> list = new ArrayList<>(strings.length);
                for (int i = 0; i < strings.length; i++) {
                    list.add(Integer.decode(strings[i]));
                }
                intent.putExtra(key, list);
            }
            break;
            case "--el": {
                std::string key = cmd.getNextArgRequired();
                std::string value = cmd.getNextArgRequired();
                intent.putExtra(key, Long.valueOf(value));
            }
            break;
            case "--ela": {
                std::string key = cmd.getNextArgRequired();
                std::string value = cmd.getNextArgRequired();
                std::string[] strings = value.split(",");
                long[] list = new long[strings.length];
                for (int i = 0; i < strings.length; i++) {
                    list[i] = Long.valueOf(strings[i]);
                }
                intent.putExtra(key, list);
                hasIntentInfo = true;
            }
            break;
            case "--elal": {
                std::string key = cmd.getNextArgRequired();
                std::string value = cmd.getNextArgRequired();
                std::string[] strings = value.split(",");
                ArrayList<Long> list = new ArrayList<>(strings.length);
                for (int i = 0; i < strings.length; i++) {
                    list.add(Long.valueOf(strings[i]));
                }
                intent.putExtra(key, list);
                hasIntentInfo = true;
            }
            break;
            case "--ef": {
                std::string key = cmd.getNextArgRequired();
                std::string value = cmd.getNextArgRequired();
                intent.putExtra(key, Float.valueOf(value));
                hasIntentInfo = true;
            }
            break;
            case "--efa": {
                std::string key = cmd.getNextArgRequired();
                std::string value = cmd.getNextArgRequired();
                std::string[] strings = value.split(",");
                float[] list = new float[strings.length];
                for (int i = 0; i < strings.length; i++) {
                    list[i] = Float.valueOf(strings[i]);
                }
                intent.putExtra(key, list);
                hasIntentInfo = true;
            }
            break;
            case "--efal": {
                std::string key = cmd.getNextArgRequired();
                std::string value = cmd.getNextArgRequired();
                std::string[] strings = value.split(",");
                ArrayList<Float> list = new ArrayList<>(strings.length);
                for (int i = 0; i < strings.length; i++) {
                    list.add(Float.valueOf(strings[i]));
                }
                intent.putExtra(key, list);
                hasIntentInfo = true;
            }
            break;
            case "--esa": {
                std::string key = cmd.getNextArgRequired();
                std::string value = cmd.getNextArgRequired();
                // Split on commas unless they are preceeded by an escape.
                // The escape character must be escaped for the string and
                // again for the regex, thus four escape characters become one.
                std::string[] strings = value.split("(?<!\\\\),");
                intent.putExtra(key, strings);
                hasIntentInfo = true;
            }
            break;
            case "--esal": {
                std::string key = cmd.getNextArgRequired();
                std::string value = cmd.getNextArgRequired();
                // Split on commas unless they are preceeded by an escape.
                // The escape character must be escaped for the string and
                // again for the regex, thus four escape characters become one.
                std::string[] strings = value.split("(?<!\\\\),");
                ArrayList<std::string> list = new ArrayList<>(strings.length);
                for (int i = 0; i < strings.length; i++) {
                    list.add(strings[i]);
                }
                intent.putExtra(key, list);
                hasIntentInfo = true;
            }
            break;
            case "--ez": {
                std::string key = cmd.getNextArgRequired();
                std::string value = cmd.getNextArgRequired().toLowerCase();
                // Boolean.valueOf() results in false for anything that is not "true", which is
                // error-prone in shell commands
                bool arg;
                if ("true".equals(value) || "t".equals(value)) {
                    arg = true;
                } else if ("false".equals(value) || "f".equals(value)) {
                    arg = false;
                } else {
                    try {
                        arg = Integer.decode(value) != 0;
                    } catch (NumberFormatException ex) {
                        throw new IllegalArgumentException("Invalid bool value: " + value);
                    }
                }

                intent.putExtra(key, arg);
            }
            break;
            case "-n": {
                std::string str = cmd.getNextArgRequired();
                ComponentName cn = ComponentName.unflattenFromString(str);
                if (cn == nullptr)
                    throw new IllegalArgumentException("Bad component name: " + str);
                intent.setComponent(cn);
                if (intent == baseIntent) {
                    hasIntentInfo = true;
                }
            }
            break;
            case "-p": {
                std::string str = cmd.getNextArgRequired();
                intent.setPackage(str);
                if (intent == baseIntent) {
                    hasIntentInfo = true;
                }
            }
            break;
            case "-f":
                std::string str = cmd.getNextArgRequired();
                intent.setFlags(Integer.decode(str).intValue());
                break;
            case "--grant-read-uri-permission":
                intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                break;
            case "--grant-write-uri-permission":
                intent.addFlags(Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
                break;
            case "--grant-persistable-uri-permission":
                intent.addFlags(Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION);
                break;
            case "--grant-prefix-uri-permission":
                intent.addFlags(Intent.FLAG_GRANT_PREFIX_URI_PERMISSION);
                break;
            case "--exclude-stopped-packages":
                intent.addFlags(Intent.FLAG_EXCLUDE_STOPPED_PACKAGES);
                break;
            case "--include-stopped-packages":
                intent.addFlags(Intent.FLAG_INCLUDE_STOPPED_PACKAGES);
                break;
            case "--debug-log-resolution":
                intent.addFlags(Intent.FLAG_DEBUG_LOG_RESOLUTION);
                break;
            case "--activity-brought-to-front":
                intent.addFlags(Intent.FLAG_ACTIVITY_BROUGHT_TO_FRONT);
                break;
            case "--activity-clear-top":
                intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
                break;
            case "--activity-clear-when-task-reset":
                intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);
                break;
            case "--activity-exclude-from-recents":
                intent.addFlags(Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
                break;
            case "--activity-launched-from-history":
                intent.addFlags(Intent.FLAG_ACTIVITY_LAUNCHED_FROM_HISTORY);
                break;
            case "--activity-multiple-task":
                intent.addFlags(Intent.FLAG_ACTIVITY_MULTIPLE_TASK);
                break;
            case "--activity-no-animation":
                intent.addFlags(Intent.FLAG_ACTIVITY_NO_ANIMATION);
                break;
            case "--activity-no-history":
                intent.addFlags(Intent.FLAG_ACTIVITY_NO_HISTORY);
                break;
            case "--activity-no-user-action":
                intent.addFlags(Intent.FLAG_ACTIVITY_NO_USER_ACTION);
                break;
            case "--activity-previous-is-top":
                intent.addFlags(Intent.FLAG_ACTIVITY_PREVIOUS_IS_TOP);
                break;
            case "--activity-reorder-to-front":
                intent.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
                break;
            case "--activity-reset-task-if-needed":
                intent.addFlags(Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED);
                break;
            case "--activity-single-top":
                intent.addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP);
                break;
            case "--activity-clear-task":
                intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
                break;
            case "--activity-task-on-home":
                intent.addFlags(Intent.FLAG_ACTIVITY_TASK_ON_HOME);
                break;
            case "--activity-match-external":
                intent.addFlags(Intent.FLAG_ACTIVITY_MATCH_EXTERNAL);
                break;
            case "--receiver-registered-only":
                intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY);
                break;
            case "--receiver-replace-pending":
                intent.addFlags(Intent.FLAG_RECEIVER_REPLACE_PENDING);
                break;
            case "--receiver-foreground":
                intent.addFlags(Intent.FLAG_RECEIVER_FOREGROUND);
                break;
            case "--receiver-no-abort":
                intent.addFlags(Intent.FLAG_RECEIVER_NO_ABORT);
                break;
            case "--receiver-include-background":
                intent.addFlags(Intent.FLAG_RECEIVER_INCLUDE_BACKGROUND);
                break;
            case "--selector":
                intent.setDataAndType(data, type);
                intent = new Intent();
                break;
            default:
                if (optionHandler != null && optionHandler.handleOption(opt, cmd)) {
                    // Okay, caller handled this option.
                } else {
                    throw new IllegalArgumentException("Unknown option: " + opt);
                }
                break;
        }
    }
    intent.setDataAndType(data, type);

    final bool hasSelector = intent != baseIntent;
    if (hasSelector) {
        // A selector was specified; fix up.
        baseIntent.setSelector(intent);
        intent = baseIntent;
    }

    std::string arg = cmd.getNextArg();
    baseIntent = null;
    if (arg == nullptr) {
        if (hasSelector) {
            // If a selector has been specified, and no arguments
            // have been supplied for the main Intent, then we can
            // assume it is ACTION_MAIN CATEGORY_LAUNCHER; we don't
            // need to have a component name specified yet, the
            // selector will take care of that.
            baseIntent = new Intent(Intent.ACTION_MAIN);
            baseIntent.addCategory(Intent.CATEGORY_LAUNCHER);
        }
    } else if (arg.indexOf(':') >= 0) {
        // The argument is a URI.  Fully parse it, and use that result
        // to fill in any data not specified so far.
        baseIntent = Intent.parseUri(arg, Intent.URI_INTENT_SCHEME
                | Intent.URI_ANDROID_APP_SCHEME | Intent.URI_ALLOW_UNSAFE);
    } else if (arg.indexOf('/') >= 0) {
        // The argument is a component name.  Build an Intent to launch
        // it.
        baseIntent = new Intent(Intent.ACTION_MAIN);
        baseIntent.addCategory(Intent.CATEGORY_LAUNCHER);
        baseIntent.setComponent(ComponentName.unflattenFromString(arg));
    } else {
        // Assume the argument is a package name.
        baseIntent = new Intent(Intent.ACTION_MAIN);
        baseIntent.addCategory(Intent.CATEGORY_LAUNCHER);
        baseIntent.setPackage(arg);
    }
    if (baseIntent != nullptr) {
        Bundle extras = intent.getExtras();
        intent.replaceExtras((Bundle)nullptr);
        Bundle uriExtras = baseIntent.getExtras();
        baseIntent.replaceExtras((Bundle)nullptr);
        if (intent.getAction() != null && baseIntent.getCategories() != nullptr) {
            HashSet<std::string> cats = new HashSet<std::string>(baseIntent.getCategories());
            for (std::string c : cats) {
                baseIntent.removeCategory(c);
            }
        }
        intent.fillIn(baseIntent, Intent.FILL_IN_COMPONENT | Intent.FILL_IN_SELECTOR);
        if (extras == nullptr) {
            extras = uriExtras;
        } else if (uriExtras != nullptr) {
            uriExtras.putAll(extras);
            extras = uriExtras;
        }
        intent.replaceExtras(extras);
        hasIntentInfo = true;
    }

    if (!hasIntentInfo) throw new IllegalArgumentException("No intent supplied");
    return intent;
}

void Intent::printIntentArgsHelp(PrintWriter pw, std::string prefix) {
    final std::string[] lines = new std::string[] {
            "<INTENT> specifications include these flags and arguments:",
            "    [-a <ACTION>] [-d <DATA_URI>] [-t <MIME_TYPE>]",
            "    [-c <CATEGORY> [-c <CATEGORY>] ...]",
            "    [-n <COMPONENT_NAME>]",
            "    [-e|--es <EXTRA_KEY> <EXTRA_STRING_VALUE> ...]",
            "    [--esn <EXTRA_KEY> ...]",
            "    [--ez <EXTRA_KEY> <EXTRA_BOOLEAN_VALUE> ...]",
            "    [--ei <EXTRA_KEY> <EXTRA_INT_VALUE> ...]",
            "    [--el <EXTRA_KEY> <EXTRA_LONG_VALUE> ...]",
            "    [--ef <EXTRA_KEY> <EXTRA_FLOAT_VALUE> ...]",
            "    [--eu <EXTRA_KEY> <EXTRA_URI_VALUE> ...]",
            "    [--ecn <EXTRA_KEY> <EXTRA_COMPONENT_NAME_VALUE>]",
            "    [--eia <EXTRA_KEY> <EXTRA_INT_VALUE>[,<EXTRA_INT_VALUE...]]",
            "        (mutiple extras passed as Integer[])",
            "    [--eial <EXTRA_KEY> <EXTRA_INT_VALUE>[,<EXTRA_INT_VALUE...]]",
            "        (mutiple extras passed as List<Integer>)",
            "    [--ela <EXTRA_KEY> <EXTRA_LONG_VALUE>[,<EXTRA_LONG_VALUE...]]",
            "        (mutiple extras passed as Long[])",
            "    [--elal <EXTRA_KEY> <EXTRA_LONG_VALUE>[,<EXTRA_LONG_VALUE...]]",
            "        (mutiple extras passed as List<Long>)",
            "    [--efa <EXTRA_KEY> <EXTRA_FLOAT_VALUE>[,<EXTRA_FLOAT_VALUE...]]",
            "        (mutiple extras passed as Float[])",
            "    [--efal <EXTRA_KEY> <EXTRA_FLOAT_VALUE>[,<EXTRA_FLOAT_VALUE...]]",
            "        (mutiple extras passed as List<Float>)",
            "    [--esa <EXTRA_KEY> <EXTRA_STRING_VALUE>[,<EXTRA_STRING_VALUE...]]",
            "        (mutiple extras passed as std::string[]; to embed a comma into a string,",
            "         escape it using \"\\,\")",
            "    [--esal <EXTRA_KEY> <EXTRA_STRING_VALUE>[,<EXTRA_STRING_VALUE...]]",
            "        (mutiple extras passed as List<std::string>; to embed a comma into a string,",
            "         escape it using \"\\,\")",
            "    [-f <FLAG>]",
            "    [--grant-read-uri-permission] [--grant-write-uri-permission]",
            "    [--grant-persistable-uri-permission] [--grant-prefix-uri-permission]",
            "    [--debug-log-resolution] [--exclude-stopped-packages]",
            "    [--include-stopped-packages]",
            "    [--activity-brought-to-front] [--activity-clear-top]",
            "    [--activity-clear-when-task-reset] [--activity-exclude-from-recents]",
            "    [--activity-launched-from-history] [--activity-multiple-task]",
            "    [--activity-no-animation] [--activity-no-history]",
            "    [--activity-no-user-action] [--activity-previous-is-top]",
            "    [--activity-reorder-to-front] [--activity-reset-task-if-needed]",
            "    [--activity-single-top] [--activity-clear-task]",
            "    [--activity-task-on-home] [--activity-match-external]",
            "    [--receiver-registered-only] [--receiver-replace-pending]",
            "    [--receiver-foreground] [--receiver-no-abort]",
            "    [--receiver-include-background]",
            "    [--selector]",
            "    [<URI> | <PACKAGE> | <COMPONENT>]"
    };
    for (std::string line : lines) {
        pw.print(prefix);
        pw.println(line);
    }
}
#endif

std::string Intent::getAction() const{
    return mAction;
}

Uri* Intent::getData() const{
    return mData;
}

std::string Intent::getDataString() const{
    return mData != nullptr ? mData->toString() : "";
}

std::string Intent::getScheme() const{
    return mData != nullptr ? mData->getScheme() : "";
}

std::string Intent::getType() const{
    return mType;
}

std::string Intent::resolveType(Context* context) {
    return "";//resolveType(context->getContentResolver());
}

/*std::string Intent::resolveType(ContentResolver resolver) {
    if (mType != nullptr) {
        return mType;
    }
    if (mData != nullptr) {
        if ("content".equals(mData->getScheme())) {
            return resolver.getType(mData);
        }
    }
    return null;
}

std::string Intent::resolveTypeIfNeeded(ContentResolver resolver) {
    if (mComponent != nullptr) {
        return mType;
    }
    return resolveType(resolver);
}*/

bool Intent::hasCategory(const std::string& category) {
    return /*mCategories != nullptr &&*/ mCategories.count(category);
}

std::set<std::string> Intent::getCategories() {
    return mCategories;
}

Intent* Intent::getSelector() {
    return mSelector;
}

ClipData* Intent::getClipData() {
    return mClipData;
}

int Intent::getContentUserHint() {
    return mContentUserHint;
}

std::string Intent::getLaunchToken() {
    return mLaunchToken;
}

void Intent::setLaunchToken(const std::string& launchToken) {
    mLaunchToken = launchToken;
}

/*void setExtrasClassLoader(ClassLoader loader) {
    if (mExtras != nullptr) {
        mExtras.setClassLoader(loader);
    }
}*/

bool Intent::hasExtra(const std::string& name) {
    return mExtras != nullptr && mExtras->containsKey(name);
}

bool Intent::hasFileDescriptors() {
    return mExtras != nullptr && false;//mExtras->hasFileDescriptors();
}

void Intent::setAllowFds(bool allowFds) {
    if (mExtras != nullptr) {
        //mExtras->setAllowFds(allowFds);
    }
}

void Intent::setDefusable(bool defusable) {
    if (mExtras != nullptr) {
        //mExtras->setDefusable(defusable);
    }
}

bool Intent::getBooleanExtra(const std::string& name, bool defaultValue) {
    return mExtras == nullptr ? defaultValue : mExtras->getValue<bool>(name,defaultValue);
}

uint8_t Intent::getByteExtra(const std::string& name, uint8_t defaultValue) {
    return mExtras == nullptr ? defaultValue : mExtras->getValue<uint8_t>(name,defaultValue);
}

short Intent::getShortExtra(const std::string& name, short defaultValue) {
    return mExtras == nullptr ? defaultValue : mExtras->getValue<int16_t>(name, defaultValue);
}

char Intent::getCharExtra(const std::string& name, char defaultValue) {
    return mExtras == nullptr ? defaultValue : mExtras->getValue<char>(name, defaultValue);
}

int Intent::getIntExtra(const std::string& name, int defaultValue) {
    return mExtras == nullptr ? defaultValue : mExtras->getValue<int32_t>(name, defaultValue);
}

int64_t Intent::getLongExtra(const std::string& name, int64_t defaultValue) {
    return mExtras == nullptr ? defaultValue : mExtras->getValue<int64_t>(name, defaultValue);
}

float Intent::getFloatExtra(const std::string& name, float defaultValue) {
    return mExtras == nullptr ? defaultValue : mExtras->getValue<float>(name, defaultValue);
}

double Intent::getDoubleExtra(const std::string& name, double defaultValue) {
    return mExtras == nullptr ? defaultValue : mExtras->getValue<double>(name, defaultValue);
}

std::string Intent::getStringExtra(const std::string& name) {
    return mExtras == nullptr ? nullptr : mExtras->getValue<std::string>(name);
}

Parcelable* Intent::getParcelableExtra(const std::string& name) {
    return mExtras == nullptr ? nullptr : nullptr;//mExtras->getParcelable(name);
}

std::vector<Parcelable*> Intent::getParcelableArrayExtra(const std::string& name) {
    return std::vector<Parcelable*>();//return mExtras == nullptr ? nullptr : mExtras->getParcelableArray(name);
}

/*<T extends Parcelable> ArrayList<T> getParcelableArrayListExtra(const std::string&) {
    return mExtras == null ? null : mExtras.<T>getParcelableArrayList(name);
}

Serializable Intent::getSerializableExtra(const std::string&) {
    return mExtras == nullptr ? nullptr : mExtras->getSerializable(name);
}*/

std::vector<int32_t> Intent::getIntegerArrayListExtra(const std::string& name) {
    //return mExtras == nullptr ? nullptr : mExtras->getIntegerArrayList(name);
    return std::vector<int32_t>();
}

std::vector<std::string> getStringArrayListExtra(const std::string& name) {
    //return mExtras == nullptr ? nullptr : mExtras->getStringArrayList(name);
    return std::vector<std::string>();
}

std::vector<bool> Intent::getBooleanArrayExtra(const std::string& name) {
    return mExtras == nullptr ? std::vector<bool>() : mExtras->getBooleanArray(name);
    return std::vector<bool>();
}

std::vector<int8_t> Intent::getByteArrayExtra(const std::string& name) {
    return mExtras == nullptr ? std::vector<int8_t>() : mExtras->getByteArray(name);
}

std::vector<int16_t> Intent::getShortArrayExtra(const std::string& name) {
    return mExtras == nullptr ? std::vector<int16_t>() : mExtras->getShortArray(name);
}

/*char[] Intent::getCharArrayExtra(const std::string& name) {
    return mExtras == nullptr ? nullptr : mExtras->getCharArray(name);
}*/

std::vector<int> Intent::getIntArrayExtra(const std::string& name) {
    return mExtras == nullptr ? std::vector<int>() : mExtras->getIntArray(name);
}

std::vector<int64_t> Intent::getLongArrayExtra(const std::string& name) {
    return mExtras == nullptr ? std::vector<int64_t>() : mExtras->getLongArray(name);
}

std::vector<float> Intent::getFloatArrayExtra(const std::string& name) {
    return mExtras == nullptr ? std::vector<float>() : mExtras->getFloatArray(name);
}

std::vector<double> Intent::getDoubleArrayExtra(const std::string& name) {
    return mExtras == nullptr ? std::vector<double>() : mExtras->getDoubleArray(name);
}

std::vector<std::string> Intent::getStringArrayExtra(const std::string& name) {
    return mExtras == nullptr ? std::vector<std::string>() : mExtras->getStringArray(name);
}

Bundle* Intent::getBundleExtra(const std::string& name) {
    return mExtras == nullptr ? nullptr : nullptr;//mExtras->getBundle(name);
}

Bundle* Intent::getExtras() {
    return (mExtras != nullptr) ? new Bundle(*mExtras) : nullptr;
}

void Intent::removeUnsafeExtras() {
    if (mExtras != nullptr) {
        mExtras = nullptr;//mExtras->filterValues();
    }
}

bool Intent::canStripForHistory() const{
    return false;//((mExtras != nullptr) && mExtras->isParcelled()) || (mClipData != nullptr);
}

Intent* Intent::maybeStripForHistory() {
    // TODO Scan and remove possibly heavy instances like Bitmaps from unparcelled extras?

    if (!canStripForHistory()) {
        return this;
    }
    return new Intent(*this, COPY_MODE_HISTORY);
}

int Intent::getFlags() const{
    return mFlags;
}

bool Intent::isExcludingStopped() const{
    return (mFlags&(FLAG_EXCLUDE_STOPPED_PACKAGES|FLAG_INCLUDE_STOPPED_PACKAGES))
            == FLAG_EXCLUDE_STOPPED_PACKAGES;
}

std::string Intent::getPackage() const{
    return mPackage;
}
#if 0
ComponentName Intet::getComponent() {
    return mComponent;
}

Rect getSourceBounds() {
    return mSourceBounds;
}

ComponentName Intent::resolveActivity(PackageManager pm) {
    if (mComponent != nullptr) {
        return mComponent;
    }

    ResolveInfo info = pm.resolveActivity(
        this, PackageManager.MATCH_DEFAULT_ONLY);
    if (info != nullptr) {
        return new ComponentName(
                info.activityInfo.applicationInfo.packageName,
                info.activityInfo.name);
    }

    return null;
}

ActivityInfo Intent::resolveActivityInfo(PackageManager pm,int flags) {
    ActivityInfo ai = null;
    if (mComponent != nullptr) {
        try {
            ai = pm.getActivityInfo(mComponent, flags);
        } catch (PackageManager.NameNotFoundException e) {
            // ignore
        }
    } else {
        ResolveInfo info = pm.resolveActivity(
            this, PackageManager.MATCH_DEFAULT_ONLY | flags);
        if (info != nullptr) {
            ai = info.activityInfo;
        }
    }

    return ai;
}

ComponentName Intent::resolveSystemService(PackageManager pm,int flags) {
    if (mComponent != nullptr) {
        return mComponent;
    }

    List<ResolveInfo> results = pm.queryIntentServices(this, flags);
    if (results == nullptr) {
        return null;
    }
    ComponentName comp = null;
    for (int i=0; i<results.size(); i++) {
        ResolveInfo ri = results.get(i);
        if ((ri.serviceInfo.applicationInfo.flags&ApplicationInfo.FLAG_SYSTEM) == 0) {
            continue;
        }
        ComponentName foundComp = new ComponentName(ri.serviceInfo.applicationInfo.packageName,
                ri.serviceInfo.name);
        if (comp != nullptr) {
            throw new IllegalStateException("Multiple system services handle " + this
                    + ": " + comp + ", " + foundComp);
        }
        comp = foundComp;
    }
    return comp;
}
#endif

Intent& Intent::setAction(const std::string& action) {
    mAction = action;//action != null ? action.intern() : null;
    return *this;
}

Intent& Intent::setData(Uri* data) {
    mData = data;
    mType.clear();
    return *this;
}

Intent& Intent::setDataAndNormalize(Uri& data) {
    return *this;//setData(data->normalizeScheme());
}

Intent& Intent::setType(const std::string& type) {
    mData = nullptr;
    mType = type;
    return *this;
}

Intent& Intent::setTypeAndNormalize(const std::string& type) {
    return setType(normalizeMimeType(type));
}

Intent& Intent::setDataAndType(Uri* data,const std::string& type) {
    mData = data;
    mType = type;
    return *this;
}

Intent& Intent::setDataAndTypeAndNormalize(Uri& data,const std::string& type) {
    return *this;//setDataAndType(data.normalizeScheme(), normalizeMimeType(type));
}

Intent& Intent::addCategory(const std::string& category) {
    mCategories.emplace(category);
    return *this;
}

void Intent::removeCategory(const std::string& category) {
    if (mCategories.size()) {
        auto it = mCategories.find(category);
        if(it!=mCategories.end()){
            mCategories.erase(it);
        }
    }
}

void Intent::setSelector(Intent* selector) {
    if (selector == this) {
        throw std::logic_error("Intent being set as a selector of itself");
    }
    if (selector != nullptr && mPackage.size()) {
        throw std::logic_error("Can't set selector when package name is already set");
    }
    mSelector = selector;
}

void Intent::setClipData(ClipData* clip) {
    mClipData = clip;
}

void Intent::prepareToLeaveUser(int userId) {
    // If mContentUserHint is not UserHandle.USER_CURRENT, the intent has already left a user.
    // We want mContentUserHint to refer to the original user, so don't do anything.
    /*if (mContentUserHint == UserHandle.USER_CURRENT) {
        mContentUserHint = userId;
    }*/
}

Intent& Intent::putExtra(const std::string& name, bool value) {
    if (mExtras == nullptr) {
        mExtras = new Bundle();
    }
    mExtras->putBoolean(name, value);
    return *this;
}

Intent& Intent::putExtra(const std::string& name, int8_t value) {
    if (mExtras == nullptr) {
        mExtras = new Bundle();
    }
    mExtras->putByte(name, value);
    return *this;
}

/*Intent& Intent::putExtra(const std::string& name, char value) {
    if (mExtras == nullptr) {
        mExtras = new Bundle();
    }
    mExtras->putChar(name, value);
    return *this;
}*/

Intent& Intent::putExtra(const std::string& name, int16_t value) {
    if (mExtras == nullptr) {
        mExtras = new Bundle();
    }
    mExtras->putShort(name, value);
    return *this;
}

Intent& Intent::putExtra(const std::string& name, int value) {
    if (mExtras == nullptr) {
        mExtras = new Bundle();
    }
    mExtras->putInt(name, value);
    return *this;
}

Intent& Intent::putExtra(const std::string& name, int64_t value) {
    if (mExtras == nullptr) {
        mExtras = new Bundle();
    }
    mExtras->putLong(name, value);
    return *this;
}

Intent& Intent::putExtra(const std::string& name, float value) {
    if (mExtras == nullptr) {
        mExtras = new Bundle();
    }
    mExtras->putFloat(name, value);
    return *this;
}


Intent& Intent::putExtra(const std::string& name, double value) {
    if (mExtras == nullptr) {
        mExtras = new Bundle();
    }
    mExtras->putDouble(name, value);
    return *this;
}

Intent& Intent::putExtra(const std::string& name,const std::string& value) {
    if (mExtras == nullptr) {
        mExtras = new Bundle();
    }
    mExtras->putString(name, value);
    return *this;
}

Intent& Intent::putExtra(const std::string& name, Parcelable* value) {
    if (mExtras == nullptr) {
        mExtras = new Bundle();
    }
    //mExtras->putParcelable(name, value);
    return *this;
}

Intent& Intent::putExtra(const std::string& name, const std::vector<Parcelable*>& value) {
    if (mExtras == nullptr) {
        mExtras = new Bundle();
    }
    //mExtras->putParcelableArray(name, value);
    return *this;
}

/*Intent& Intent::putParcelableArrayListExtra(const std::string&,ArrayList<? extends Parcelable> value) {
    if (mExtras == nullptr) {
        mExtras = new Bundle();
    }
    mExtras->putParcelableArrayList(name, value);
    return *this;
}*/

Intent& Intent::putIntegerArrayListExtra(const std::string& name,const std::vector<int>& value) {
    if (mExtras == nullptr) {
        mExtras = new Bundle();
    }
    mExtras->putIntArray(name, value);
    return *this;
}

Intent& Intent::putStringArrayListExtra(const std::string& name, const std::vector<std::string>& value) {
    if (mExtras == nullptr) {
        mExtras = new Bundle();
    }
    mExtras->putStringArray(name, value);
    return *this;
}

/*Intent& Intent::putExtra(const std::string& name, Serializable value) {
    if (mExtras == nullptr) {
        mExtras = new Bundle();
    }
    mExtras->putSerializable(name, value);
    return *this;
}*/

Intent& Intent::putExtra(const std::string& name, const std::vector<bool>& value) {
    if (mExtras == nullptr) {
        mExtras = new Bundle();
    }
    mExtras->putBooleanArray(name, value);
    return *this;
}

Intent& Intent::putExtra(const std::string& name,const std::vector<int8_t>& value) {
    if (mExtras == nullptr) {
        mExtras = new Bundle();
    }
    mExtras->putByteArray(name, value);
    return *this;
}

Intent& Intent::putExtra(const std::string& name, const std::vector<int16_t>& value) {
    if (mExtras == nullptr) {
        mExtras = new Bundle();
    }
    mExtras->putShortArray(name, value);
    return *this;
}

/*Intent& Intent::putExtra(const std::string& name, char[] value) {
    if (mExtras == nullptr) {
        mExtras = new Bundle();
    }
    mExtras.putCharArray(name, value);
    return this;
}*/

Intent& Intent::putExtra(const std::string& name, const std::vector<int32_t>& value) {
    if (mExtras == nullptr) {
        mExtras = new Bundle();
    }
    mExtras->putIntArray(name, value);
    return *this;
}

Intent& Intent::putExtra(const std::string& name, const std::vector<int64_t>& value) {
    if (mExtras == nullptr) {
        mExtras = new Bundle();
    }
    mExtras->putLongArray(name, value);
    return *this;
}

Intent& Intent::putExtra(const std::string& name, const std::vector<float>& value) {
    if (mExtras == nullptr) {
        mExtras = new Bundle();
    }
    mExtras->putFloatArray(name, value);
    return *this;
}

Intent& Intent::putExtra(const std::string& name,const std::vector<double>& value) {
    if (mExtras == nullptr) {
        mExtras = new Bundle();
    }
    mExtras->putDoubleArray(name, value);
    return *this;
}


Intent& Intent::putExtra(const std::string& name, const std::vector<std::string>& value) {
    if (mExtras == nullptr) {
        mExtras = new Bundle();
    }
    mExtras->putStringArray(name, value);
    return *this;
}

/*Intent& Intent::putExtra(const std::string& name, CharSequence[] value) {
    if (mExtras == nullptr) {
        mExtras = new Bundle();
    }
    mExtras->putCharSequenceArray(name, value);
    return *this;
}*/


Intent& Intent::putExtra(const std::string& name, Bundle* value) {
    if (mExtras == nullptr) {
        mExtras = new Bundle();
    }
    //mExtras->putBundle(name, value);
    return *this;
}

Intent& Intent::putExtras(const Intent& src) {
    if (src.mExtras != nullptr) {
        if (mExtras == nullptr) {
            mExtras = new Bundle(*src.mExtras);
        } else {
            //mExtras->putAll(src.mExtras);
        }
    }
    return *this;
}

Intent& Intent::putExtras(const Bundle* extras) {
    if (mExtras == nullptr) {
        mExtras = new Bundle();
    }
    //mExtras.putAll(extras);
    return *this;
}

Intent& Intent::replaceExtras(const Intent& src) {
    mExtras = src.mExtras != nullptr ? new Bundle(*src.mExtras) : nullptr;
    return *this;
}

Intent& Intent::replaceExtras(const Bundle* extras) {
    mExtras = extras != nullptr ? new Bundle(*extras) : nullptr;
    return *this;
}

void Intent::removeExtra(const std::string& name) {
    if (mExtras != nullptr) {
        mExtras->remove(name);
        if (mExtras->size() == 0) {
            //mExtras = null;
        }
    }
}

Intent& Intent::setFlags(int flags) {
    mFlags = flags;
    return *this;
}

Intent& Intent::addFlags(int flags) {
    mFlags |= flags;
    return *this;
}

void Intent::removeFlags(int flags) {
    mFlags &= ~flags;
}

Intent& Intent::setPackage(const std::string& packageName) {
    if (packageName.size() && mSelector != nullptr) {
        throw std::logic_error("Can't set package name when selector is already set");
    }
    mPackage = packageName;
    return *this;
}

/*Intent& Intent::setComponent(ComponentName component) {
    mComponent = component;
    return *this;
}

Intent& Intent::setClassName(Context* packageContext,const std::string& className) {
    mComponent = new ComponentName(packageContext, className);
    return *this;
}

Intent& Intent::setClassName(const std::string& packageName, const std::string& className) {
    mComponent = new ComponentName(packageName, className);
    return *this;
}

Intent& setClass(Context* packageContext, Class<?> cls) {
    mComponent = new ComponentName(packageContext, cls);
    return *this;
}*/

void Intent::setSourceBounds(const Rect* r) {
    if (r != nullptr) {
        mSourceBounds = *r;// = new Rect(r);
    } else {
        mSourceBounds.set(0,0,0,0);// = null;
    }
}

int Intent::fillIn(const Intent& other, int flags) {
    int changes = 0;
    bool mayHaveCopiedUris = false;
    if (other.mAction.size()
            && (mAction.empty() || (flags&FILL_IN_ACTION) != 0)) {
        mAction = other.mAction;
        changes |= FILL_IN_ACTION;
    }
    if ((other.mData!=nullptr || other.mType.size())
            && ((mData==nullptr && mType.empty())
                    || (flags&FILL_IN_DATA) != 0)) {
        mData = other.mData;
        mType = other.mType;
        changes |= FILL_IN_DATA;
        mayHaveCopiedUris = true;
    }
    if (other.mCategories.size()
            && (mCategories.empty() || (flags&FILL_IN_CATEGORIES) != 0)) {
        if (other.mCategories.size()) {
            mCategories=other.mCategories;
        }
        changes |= FILL_IN_CATEGORIES;
    }
    if (other.mPackage.size()
            && (mPackage.empty() || (flags&FILL_IN_PACKAGE) != 0)) {
        // Only do this if mSelector is not set.
        if (mSelector == nullptr) {
            mPackage = other.mPackage;
            changes |= FILL_IN_PACKAGE;
        }
    }
    // Selector is special: it can only be set if explicitly allowed,
    // for the same reason as the component name.
    if (other.mSelector != nullptr && (flags&FILL_IN_SELECTOR) != 0) {
        if (mPackage.empty()) {
            mSelector = new Intent(*other.mSelector);
            mPackage.clear();// = null;
            changes |= FILL_IN_SELECTOR;
        }
    }
    if (other.mClipData != nullptr
            && (mClipData == nullptr || (flags&FILL_IN_CLIP_DATA) != 0)) {
        mClipData = other.mClipData;
        changes |= FILL_IN_CLIP_DATA;
        mayHaveCopiedUris = true;
    }
#if 0
    // Component is special: it can -only- be set if explicitly allowed,
    // since otherwise the sender could force the intent somewhere the
    // originator didn't intend.
    if (other.mComponent != nullptr && (flags&FILL_IN_COMPONENT) != 0) {
        mComponent = other.mComponent;
        changes |= FILL_IN_COMPONENT;
    }
    mFlags |= other.mFlags;
    if (other.mSourceBounds != nullptr
            && (mSourceBounds == nullptr || (flags&FILL_IN_SOURCE_BOUNDS) != 0)) {
        mSourceBounds = new Rect(other.mSourceBounds);
        changes |= FILL_IN_SOURCE_BOUNDS;
    }
    if (mExtras == nullptr) {
        if (other.mExtras != nullptr) {
            mExtras = new Bundle(other.mExtras);
            mayHaveCopiedUris = true;
        }
    } else if (other.mExtras != nullptr) {
        try {
            Bundle newb = new Bundle(other.mExtras);
            newb.putAll(mExtras);
            mExtras = newb;
            mayHaveCopiedUris = true;
        } catch (std::exception& e) {
            // Modifying the extras can cause us to unparcel the contents
            // of the bundle, and if we do this in the system process that
            // may fail.  We really should handle this (i.e., the Bundle
            // impl shouldn't be on top of a plain map), but for now just
            // ignore it and keep the original contents. :(
            LOGW("Failure filling in extras %s", e.what());
        }
    }
    if (mayHaveCopiedUris && mContentUserHint == UserHandle.USER_CURRENT
            && other.mContentUserHint != UserHandle.USER_CURRENT) {
        mContentUserHint = other.mContentUserHint;
    }
#endif
    return changes;
}


/*static final class FilterComparison {
    private final Intent mIntent;
    private final int mHashCode;

    FilterComparison(Intent intent) {
        mIntent = intent;
        mHashCode = intent.filterHashCode();
    }

    Intent getIntent() {
        return mIntent;
    }

    @Override
    bool equals(Object obj) {
        if (obj instanceof FilterComparison) {
            Intent other = ((FilterComparison)obj).mIntent;
            return mIntent.filterEquals(other);
        }
        return false;
    }

    @Override
    int hashCode() {
        return mHashCode;
    }
}*/

bool Intent::filterEquals(const Intent* other) {
    if (other == nullptr) {
        return false;
    }

    if (mAction!=other->mAction) return false;
    if (mData!=other->mData) return false;
    if (mType!=other->mType) return false;
    if (mPackage!=other->mPackage) return false;
    //if (mComponent!=other->mComponent) return false;
    if (mCategories!=other->mCategories) return false;

    return true;
}

int Intent::filterHashCode() {
    int code = 0;
    std::hash<std::string> hasher;
    if (!mAction.empty()) {
        code += hasher(mAction);
    }
    /*if (mData != nullptr) {
        code += mData.hashCode();
    }*/
    if (!mType.empty()) {
        code += hasher(mType);
    }
    if (!mPackage.empty()) {
        code += hasher(mPackage);
    }
    /*if (mComponent != nullptr) {
        code += mComponent.hashCode();
    }
    if (mCategories != nullptr) {
        code += mCategories.hashCode();
    }*/
    return code;
}

std::string Intent::toString() {
    std::ostringstream b;

    b<<"Intent { ";
    toShortString(b, true, true, true, false);
    b<<" }";

    return b.str();
}

std::string Intent::toInsecureString() {
    std::ostringstream b;

    b<<"Intent { ";
    toShortString(b, false, true, true, false);
    b<<" }";

    return b.str();
}

std::string Intent::toInsecureStringWithClip() {
    std::ostringstream b;

    b<<"Intent { ";
    toShortString(b, false, true, true, true);
    b<<" }";

    return b.str();
}

std::string Intent::toShortString(bool secure, bool comp, bool extras, bool clip) {
    std::ostringstream b;
    toShortString(b, secure, comp, extras, clip);
    return b.str();
}

void Intent::toShortString(std::ostringstream& b, bool secure, bool comp, bool extras,bool clip) {
    bool first = true;
    if (!mAction.empty()) {
        b<<"act="<<mAction;
        first = false;
    }
    if (mCategories.size()) {
        if (!first) {
            b<<' ';
        }
        first = false;
        b<<"cat=[";
        for (auto it=mCategories.begin();it!=mCategories.end();it++){
            if (it !=mCategories.begin()) b<<',';
            b<<*it;
        }
        b<<"]";
    }
    if (mData != nullptr) {
        if (!first) {
            b<<' ';
        }
        first = false;
        b<<"dat=";
        if (secure) {
            //b<<mData->toSafeString();
        } else {
            //b.append(mData);
        }
    }
    if (!mType.empty()) {
        if (!first) {
            b<<' ';
        }
        first = false;
        b<<"typ="<<mType;
    }
    if (mFlags != 0) {
        if (!first) {
            b<<' ';
        }
        first = false;
        std::ios::fmtflags originalFlags = b.flags();
        b<<"flg=0x"<<std::hex<<mFlags;
        b.flags(originalFlags);
    }
    if (!mPackage.empty()) {
        if (!first) {
            b<<' ';
        }
        first = false;
        b<<"pkg="<<mPackage;
    }
#if 0
    if (comp && mComponent != nullptr) {
        if (!first) {
            b.append(' ');
        }
        first = false;
        b.append("cmp=").append(mComponent.flattenToShortString());
    }
    if (mSourceBounds != nullptr) {
        if (!first) {
            b.append(' ');
        }
        first = false;
        b.append("bnds=").append(mSourceBounds.toShortString());
    }
    if (mClipData != nullptr) {
        if (!first) {
            b.append(' ');
        }
        b.append("clip={");
        if (clip) {
            mClipData.toShortString(b);
        } else {
            if (mClipData.getDescription() != nullptr) {
                first = !mClipData.getDescription().toShortStringTypesOnly(b);
            } else {
                first = true;
            }
            mClipData.toShortStringShortItems(b, first);
        }
        first = false;
        b.append('}');
    }

    if (extras && mExtras != nullptr) {
        if (!first) {
            b.append(' ');
        }
        first = false;
        b.append("(has extras)");
    }
    if (mContentUserHint != UserHandle.USER_CURRENT) {
        if (!first) {
            b.append(' ');
        }
        first = false;
        b.append("u=").append(mContentUserHint);
    }
    if (mSelector != nullptr) {
        b.append(" sel=");
        mSelector.toShortString(b, secure, comp, extras, clip);
        b.append("}");
    }
#endif
}

/*void Intent::writeToProto(ProtoOutputStream proto, long fieldId) {
    // Same input parameters that toString() gives to toShortString().
    writeToProto(proto, fieldId, true, true, true, false);
}

void Intent::writeToProto(ProtoOutputStream proto, long fieldId, bool secure, bool comp,bool extras, bool clip) {
    long token = proto.start(fieldId);
    if (mAction != nullptr) {
        proto.write(IntentProto.ACTION, mAction);
    }
    if (mCategories != nullptr)  {
        for (String category : mCategories) {
            proto.write(IntentProto.CATEGORIES, category);
        }
    }
    if (mData != nullptr) {
        proto.write(IntentProto.DATA, secure ? mData.toSafeString() : mData.toString());
    }
    if (mType != nullptr) {
        proto.write(IntentProto.TYPE, mType);
    }
    if (mFlags != 0) {
        proto.write(IntentProto.FLAG, "0x" + Integer.toHexString(mFlags));
    }
    if (mPackage != nullptr) {
        proto.write(IntentProto.PACKAGE, mPackage);
    }
    if (comp && mComponent != nullptr) {
        mComponent.writeToProto(proto, IntentProto.COMPONENT);
    }
    if (mSourceBounds != nullptr) {
        proto.write(IntentProto.SOURCE_BOUNDS, mSourceBounds.toShortString());
    }
    if (mClipData != nullptr) {
        StringBuilder b = new StringBuilder();
        if (clip) {
            mClipData.toShortString(b);
        } else {
            mClipData.toShortStringShortItems(b, false);
        }
        proto.write(IntentProto.CLIP_DATA, b.toString());
    }
    if (extras && mExtras != nullptr) {
        proto.write(IntentProto.EXTRAS, mExtras.toShortString());
    }
    if (mContentUserHint != 0) {
        proto.write(IntentProto.CONTENT_USER_HINT, mContentUserHint);
    }
    if (mSelector != nullptr) {
        proto.write(IntentProto.SELECTOR, mSelector.toShortString(secure, comp, extras, clip));
    }
    proto.end(token);
}*/

std::string Intent::toURI() {
    return toUri(0);
}

std::string Intent::toUri(int flags) {
    std::ostringstream uri;
    if ((flags&URI_ANDROID_APP_SCHEME) != 0) {
        if (mPackage.empty()) {
            throw std::logic_error("Intent must include an explicit package name to build an android-app: ");
        }
        uri<<"android-app://";
        uri<<mPackage;
        std::string scheme;
        if (mData != nullptr) {
            scheme = mData->getScheme();
            if (!scheme.empty()) {
                uri<<'/'<<scheme;
                std::string authority;// = mData->getEncodedAuthority();
                if (!authority.empty()) {
                    uri<<'/'<<authority;
                    std::string path;// = mData->getEncodedPath();
                    if (!path.empty()) {
                        uri<<path;
                    }
                    std::string queryParams;// = mData->getEncodedQuery();
                    if (!queryParams.empty()) {
                        uri<<'?'<<queryParams;
                    }
                    std::string fragment;// = mData->getEncodedFragment();
                    if (!fragment.empty()) {
                        uri<<'#'<<fragment;
                    }
                }
            }
        }
        toUriFragment(uri, "", scheme.empty() ? ACTION_MAIN : ACTION_VIEW, mPackage, flags);
        return uri.str();
    }
    std::string scheme;
    if (mData != nullptr) {
        std::string data = mData->toString();
        if ((flags&URI_INTENT_SCHEME) != 0) {
            size_t N = data.length();
            for (size_t i=0; i<N; i++) {
                char c = data.at(i);
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
                        || (c >= '0' && c <= '9') || c == '.' || c == '-' || c == '+') {
                    continue;
                }
                if (c == ':' && i > 0) {
                    // Valid scheme.
                    scheme = data.substr(0, i);
                    uri<<"intent:";
                    data = data.substr(i+1);
                    break;
                }
                // No scheme.
                break;
            }
        }
        uri<<data;

    } else if ((flags&URI_INTENT_SCHEME) != 0) {
        uri<<"intent:";
    }

    toUriFragment(uri, scheme, ACTION_VIEW, nullptr, flags);

    return uri.str();
}

void Intent::toUriFragment(std::ostringstream& uri,const std::string& scheme,const std::string& defAction,const std::string& defPackage, int flags) {
    std::ostringstream frag;

    toUriInner(frag, scheme, defAction, defPackage, flags);
    if (mSelector != nullptr) {
        frag<<"SEL;";
        // Note that for now we are not going to try to handle the
        // data part; not clear how to represent this as a URI, and
        // not much utility in it.
        mSelector->toUriInner(frag, "",//mSelector->mData != nullptr ? mSelector->mData->getScheme(): "",
                "","", flags);
    }

    if (frag.str().length() > 0) {
        uri<<"#Intent;";
        uri<<frag.str();
        uri<<"end";
    }
}

void Intent::toUriInner(std::ostringstream& uri, const std::string& scheme,const std::string& defAction,const std::string& defPackage, int flags) {
    if (!scheme.empty()) {
        uri<<"scheme="<<scheme<<';';
    }
    if (mAction.size() && mAction.compare(defAction)) {
        uri<<"action="<</*Uri::encode(mAction)<<*/';';
    }
    if (mCategories.size()) {
        for (int i=0; i<mCategories.size(); i++) {
            //uri<<"category="<<Uri::encode(mCategories.at(i))<<';';
        }
    }
    if (!mType.empty()) {
        uri<<"type="<</*Uri::encode(mType, "/")<<*/';';
    }
    if (mFlags != 0) {
        std::ios::fmtflags originalFlags =uri.flags();
        uri<<"launchFlags=0x"<<std::hex<<mFlags<<';';
        uri.flags(originalFlags);
    }
    if (mPackage.size() && mPackage.compare(defPackage)) {
        uri<<"package="<</*Uri::encode(mPackage)<<*/';';
    }
#if 0
    if (mComponent != nullptr) {
        uri.append("component=").append(Uri.encode(
                mComponent.flattenToShortString(), "/")).append(';');
    }
    if (mSourceBounds != nullptr) {
        uri<<"sourceBounds=")
                .append(Uri::encode(mSourceBounds.flattenToString()))
                .append(';');
    }
    if (mExtras != nullptr) {
        for (String key : mExtras.keySet()) {
            final Object value = mExtras.get(key);
            char entryType =
                    value instanceof std::string    ? 'S' :
                    value instanceof Boolean   ? 'B' :
                    value instanceof Byte      ? 'b' :
                    value instanceof Character ? 'c' :
                    value instanceof Double    ? 'd' :
                    value instanceof Float     ? 'f' :
                    value instanceof Integer   ? 'i' :
                    value instanceof Long      ? 'l' :
                    value instanceof Short     ? 's' :
                    '\0';

            if (entryType != '\0') {
                uri.append(entryType);
                uri.append('.');
                uri.append(Uri.encode(key));
                uri.append('=');
                uri.append(Uri.encode(value.toString()));
                uri.append(';');
            }
        }
    }
#endif
}

int Intent::describeContents() {
    return 0;//(mExtras != nullptr) ? mExtras->describeContents() : 0;
}
#if 0
void Intent::writeToParcel(Parcel& out, int flags) {
    out.writeString(mAction);
    Uri::writeToParcel(out, mData);
    out.writeString(mType);
    out.writeInt(mFlags);
    out.writeString(mPackage);
    ComponentName.writeToParcel(mComponent, out);

    if (mSourceBounds != nullptr) {
        out.writeInt(1);
        mSourceBounds.writeToParcel(out, flags);
    } else {
        out.writeInt(0);
    }

    if (mCategories != nullptr) {
        final int N = mCategories.size();
        out.writeInt(N);
        for (int i=0; i<N; i++) {
            out.writeString(mCategories.valueAt(i));
        }
    } else {
        out.writeInt(0);
    }

    if (mSelector != nullptr) {
        out.writeInt(1);
        mSelector.writeToParcel(out, flags);
    } else {
        out.writeInt(0);
    }

    if (mClipData != nullptr) {
        out.writeInt(1);
        mClipData.writeToParcel(out, flags);
    } else {
        out.writeInt(0);
    }
    out.writeInt(mContentUserHint);
    out.writeBundle(mExtras);
}

static final Parcelable.Creator<Intent> CREATOR
        = new Parcelable.Creator<Intent>() {
    Intent createFromParcel(Parcel in) {
        return new Intent(in);
    }
    Intent[] newArray(int size) {
        return new Intent[size];
    }
};

Intent::Intent(Parcel& in) {
    readFromParcel(in);
}

void Intent::readFromParcel(Parcel& in) {
    setAction(in.readString());
    mData = Uri.CREATOR.createFromParcel(in);
    mType = in.readString();
    mFlags = in.readInt();
    mPackage = in.readString();
    mComponent = ComponentName.readFromParcel(in);

    if (in.readInt() != 0) {
        mSourceBounds = Rect.CREATOR.createFromParcel(in);
    }

    int N = in.readInt();
    if (N > 0) {
        mCategories = new ArraySet<String>();
        int i;
        for (i=0; i<N; i++) {
            mCategories.add(in.readString().intern());
        }
    } else {
        mCategories = null;
    }

    if (in.readInt() != 0) {
        mSelector = new Intent(in);
    }

    if (in.readInt() != 0) {
        mClipData = new ClipData(in);
    }
    mContentUserHint = in.readInt();
    mExtras = in.readBundle();
}


Intent* Intent::parseIntent(XmlPullParser& parser,const AttributeSet& attrs){
    Intent* intent = new Intent();

    intent->setAction(sa.getString("action"));

    std::string data = sa.getString("data");
    std::string mimeType = sa.getString("mimeType");
    intent.setDataAndType(data != null ? Uri.parse(data) : null, mimeType);

    std::string packageName = sa.getString("targetPackage");
    std::string className = sa.getString("targetClass");
    if (packageName != null && className != nullptr) {
        intent.setComponent(new ComponentName(packageName, className));
    }

    int outerDepth = parser.getDepth();
    int type;
    while ((type=parser.next()) != XmlPullParser::END_DOCUMENT
           && (type != XmlPullParser::END_TAG || parser.getDepth() > outerDepth)) {
        if (type == XmlPullParser::END_TAG || type == XmlPullParser::TEXT) {
            continue;
        }

        std::string nodeName = parser.getName();
        if (nodeName.equals(TAG_CATEGORIES)) {
            sa = resources.obtainAttributes(attrs, com.android.internal.R.styleable.IntentCategory);
            std::string cat = sa.getString(com.android.internal.R.styleable.IntentCategory_name);
            sa.recycle();

            if (cat != nullptr) {
                intent->addCategory(cat);
            }
            XmlUtils.skipCurrentTag(parser);

        } else if (nodeName.equals(TAG_EXTRA)) {
            if (intent.mExtras == nullptr) {
                intent.mExtras = new Bundle();
            }
            resources.parseBundleExtra(TAG_EXTRA, attrs, intent.mExtras);
            XmlUtils.skipCurrentTag(parser);

        } else {
            XmlUtils.skipCurrentTag(parser);
        }
    }

    return intent;
}

/*void Intent::saveToXml(XmlSerializer out) throws IOException {
    if (mAction != nullptr) {
        out.attribute(null, ATTR_ACTION, mAction);
    }
    if (mData != nullptr) {
        out.attribute(null, ATTR_DATA, mData.toString());
    }
    if (mType != nullptr) {
        out.attribute(null, ATTR_TYPE, mType);
    }
    if (mComponent != nullptr) {
        out.attribute(null, ATTR_COMPONENT, mComponent.flattenToShortString());
    }
    out.attribute(null, ATTR_FLAGS, Integer.toHexString(getFlags()));

    if (mCategories != nullptr) {
        out.startTag(null, TAG_CATEGORIES);
        for (int categoryNdx = mCategories.size() - 1; categoryNdx >= 0; --categoryNdx) {
            out.attribute(null, ATTR_CATEGORY, mCategories.valueAt(categoryNdx));
        }
        out.endTag(null, TAG_CATEGORIES);
    }
}*/

Intent* Intent::restoreFromXml(XmlPullParser& in){
    Intent intent = new Intent();
    const int outerDepth = in.getDepth();

    int attrCount = in.getAttributeCount();
    for (int attrNdx = attrCount - 1; attrNdx >= 0; --attrNdx) {
        final std::string attrName = in.getAttributeName(attrNdx);
        final std::string attrValue = in.getAttributeValue(attrNdx);
        if (ATTR_ACTION.equals(attrName)) {
            intent.setAction(attrValue);
        } else if (ATTR_DATA.equals(attrName)) {
            intent.setData(Uri.parse(attrValue));
        } else if (ATTR_TYPE.equals(attrName)) {
            intent.setType(attrValue);
        } else if (ATTR_COMPONENT.equals(attrName)) {
            intent.setComponent(ComponentName.unflattenFromString(attrValue));
        } else if (ATTR_FLAGS.equals(attrName)) {
            intent.setFlags(Integer.parseInt(attrValue, 16));
        } else {
            Log.e("Intent", "restoreFromXml: unknown attribute=" + attrName);
        }
    }

    int event;
    std::string name;
    while (((event = in.next()) != XmlPullParser::END_DOCUMENT) &&
            (event != XmlPullParser::END_TAG || in.getDepth() < outerDepth)) {
        if (event == XmlPullParser::START_TAG) {
            name = in.getName();
            if (TAG_CATEGORIES.equals(name)) {
                attrCount = in.getAttributeCount();
                for (int attrNdx = attrCount - 1; attrNdx >= 0; --attrNdx) {
                    intent.addCategory(in.getAttributeValue(attrNdx));
                }
            } else {
                Log.w("Intent", "restoreFromXml: unknown name=" + name);
                XmlUtils.skipCurrentTag(in);
            }
        }
    }

    return intent;
}
#endif

std::string Intent::normalizeMimeType(const std::string& type) {
    /*if (type == nullptr) {
        return null;
    }

    type = type.trim().toLowerCase(Locale.ROOT);*/

    auto semicolonIndex = type.find(';');
    if (semicolonIndex != std::string::npos) {
        return type.substr(0, semicolonIndex);
    }
    return type;
}
#if 0
void Intent::prepareToLeaveProcess(Context* context) {
    const bool leavingPackage = (mComponent == nullptr)
            || !Objects.equals(mComponent.getPackageName(), context.getPackageName());
    prepareToLeaveProcess(leavingPackage);
}

void Intent::prepareToLeaveProcess(bool leavingPackage) {
    setAllowFds(false);

    if (mSelector != nullptr) {
        mSelector.prepareToLeaveProcess(leavingPackage);
    }
    if (mClipData != nullptr) {
        mClipData.prepareToLeaveProcess(leavingPackage, getFlags());
    }

    if (mExtras != null && !mExtras.isParcelled()) {
        final Object intent = mExtras.get(Intent.EXTRA_INTENT);
        if (intent instanceof Intent) {
            ((Intent) intent).prepareToLeaveProcess(leavingPackage);
        }
    }

    if (mAction != null && mData != null && StrictMode.vmFileUriExposureEnabled()
            && leavingPackage) {
        switch (mAction) {
            case ACTION_MEDIA_REMOVED:
            case ACTION_MEDIA_UNMOUNTED:
            case ACTION_MEDIA_CHECKING:
            case ACTION_MEDIA_NOFS:
            case ACTION_MEDIA_MOUNTED:
            case ACTION_MEDIA_SHARED:
            case ACTION_MEDIA_UNSHARED:
            case ACTION_MEDIA_BAD_REMOVAL:
            case ACTION_MEDIA_UNMOUNTABLE:
            case ACTION_MEDIA_EJECT:
            case ACTION_MEDIA_SCANNER_STARTED:
            case ACTION_MEDIA_SCANNER_FINISHED:
            case ACTION_MEDIA_SCANNER_SCAN_FILE:
            case ACTION_PACKAGE_NEEDS_VERIFICATION:
            case ACTION_PACKAGE_VERIFIED:
                // Ignore legacy actions
                break;
            default:
                mData.checkFileUriExposed("Intent.getData()");
        }
    }

    if (mAction != null && mData != null && StrictMode.vmContentUriWithoutPermissionEnabled()
            && leavingPackage) {
        switch (mAction) {
            case ACTION_PROVIDER_CHANGED:
            case QuickContact.ACTION_QUICK_CONTACT:
                // Ignore actions that don't need to grant
                break;
            default:
                mData.checkContentUriWithoutPermission("Intent.getData()", getFlags());
        }
    }
}

void Intent::prepareToEnterProcess() {
    // We just entered destination process, so we should be able to read all
    // parcelables inside.
    setDefusable(true);

    if (mSelector != nullptr) {
        mSelector.prepareToEnterProcess();
    }
    if (mClipData != nullptr) {
        mClipData.prepareToEnterProcess();
    }

    if (mContentUserHint != UserHandle.USER_CURRENT) {
        if (UserHandle.getAppId(Process.myUid()) != Process.SYSTEM_UID) {
            fixUris(mContentUserHint);
            mContentUserHint = UserHandle.USER_CURRENT;
        }
    }
}

bool Intent::hasWebURI() {
    if (getData() == nullptr) {
        return false;
    }
    const std::string scheme = getScheme();
    if (TextUtils.isEmpty(scheme)) {
        return false;
    }
    return false;//scheme.equals(IntentFilter.SCHEME_HTTP) || scheme.equals(IntentFilter.SCHEME_HTTPS);
}

bool Intetn::isWebIntent() {
    return ACTION_VIEW.equals(mAction)
            && hasWebURI();
}

void Intent::fixUris(int contentUserHint) {
    Uri data = getData();
    if (data != nullptr) {
        mData = maybeAddUserId(data, contentUserHint);
    }
    if (mClipData != nullptr) {
        mClipData.fixUris(contentUserHint);
    }
    std::string action = getAction();
    if (ACTION_SEND.equals(action)) {
        final Uri stream = getParcelableExtra(EXTRA_STREAM);
        if (stream != nullptr) {
            putExtra(EXTRA_STREAM, maybeAddUserId(stream, contentUserHint));
        }
    } else if (ACTION_SEND_MULTIPLE.equals(action)) {
        final ArrayList<Uri> streams = getParcelableArrayListExtra(EXTRA_STREAM);
        if (streams != nullptr) {
            ArrayList<Uri> newStreams = new ArrayList<Uri>();
            for (int i = 0; i < streams.size(); i++) {
                newStreams.add(maybeAddUserId(streams.get(i), contentUserHint));
            }
            putParcelableArrayListExtra(EXTRA_STREAM, newStreams);
        }
    } else if (MediaStore.ACTION_IMAGE_CAPTURE.equals(action)
            || MediaStore.ACTION_IMAGE_CAPTURE_SECURE.equals(action)
            || MediaStore.ACTION_VIDEO_CAPTURE.equals(action)) {
        final Uri output = getParcelableExtra(MediaStore.EXTRA_OUTPUT);
        if (output != nullptr) {
            putExtra(MediaStore.EXTRA_OUTPUT, maybeAddUserId(output, contentUserHint));
        }
    }
 }

bool Intent::migrateExtraStreamToClipData() {
    // Refuse to touch if extras already parcelled
    if (mExtras != null && mExtras.isParcelled()) return false;

    // Bail when someone already gave us ClipData
    if (getClipData() != nullptr) return false;

    final std::string action = getAction();
    if (ACTION_CHOOSER.equals(action)) {
        // Inspect contained intents to see if we need to migrate extras. We
        // don't promote ClipData to the parent, since ChooserActivity will
        // already start the picked item as the caller, and we can't combine
        // the flags in a safe way.

        bool migrated = false;
        try {
            final Intent intent = getParcelableExtra(EXTRA_INTENT);
            if (intent != nullptr) {
                migrated |= intent.migrateExtraStreamToClipData();
            }
        } catch (ClassCastException e) {
        }
        try {
            final Parcelable[] intents = getParcelableArrayExtra(EXTRA_INITIAL_INTENTS);
            if (intents != nullptr) {
                for (int i = 0; i < intents.length; i++) {
                    final Intent intent = (Intent) intents[i];
                    if (intent != nullptr) {
                        migrated |= intent.migrateExtraStreamToClipData();
                    }
                }
            }
        } catch (ClassCastException e) {
        }
        return migrated;

    } else if (ACTION_SEND.equals(action)) {
        try {
            final Uri stream = getParcelableExtra(EXTRA_STREAM);
            final CharSequence text = getCharSequenceExtra(EXTRA_TEXT);
            final std::string htmlText = getStringExtra(EXTRA_HTML_TEXT);
            if (stream != null || text != null || htmlText != nullptr) {
                final ClipData clipData = new ClipData(
                        null, new std::string[] { getType() },
                        new ClipData.Item(text, htmlText, null, stream));
                setClipData(clipData);
                addFlags(FLAG_GRANT_READ_URI_PERMISSION);
                return true;
            }
        } catch (ClassCastException e) {
        }

    } else if (ACTION_SEND_MULTIPLE.equals(action)) {
        try {
            final ArrayList<Uri> streams = getParcelableArrayListExtra(EXTRA_STREAM);
            final ArrayList<CharSequence> texts = getCharSequenceArrayListExtra(EXTRA_TEXT);
            final ArrayList<String> htmlTexts = getStringArrayListExtra(EXTRA_HTML_TEXT);
            int num = -1;
            if (streams != nullptr) {
                num = streams.size();
            }
            if (texts != nullptr) {
                if (num >= 0 && num != texts.size()) {
                    // Wha...!  F- you.
                    return false;
                }
                num = texts.size();
            }
            if (htmlTexts != nullptr) {
                if (num >= 0 && num != htmlTexts.size()) {
                    // Wha...!  F- you.
                    return false;
                }
                num = htmlTexts.size();
            }
            if (num > 0) {
                final ClipData clipData = new ClipData(
                        null, new std::string[] { getType() },
                        makeClipItem(streams, texts, htmlTexts, 0));

                for (int i = 1; i < num; i++) {
                    clipData.addItem(makeClipItem(streams, texts, htmlTexts, i));
                }

                setClipData(clipData);
                addFlags(FLAG_GRANT_READ_URI_PERMISSION);
                return true;
            }
        } catch (ClassCastException e) {
        }
    } else if (MediaStore.ACTION_IMAGE_CAPTURE.equals(action)
            || MediaStore.ACTION_IMAGE_CAPTURE_SECURE.equals(action)
            || MediaStore.ACTION_VIDEO_CAPTURE.equals(action)) {
        final Uri output;
        try {
            output = getParcelableExtra(MediaStore.EXTRA_OUTPUT);
        } catch (ClassCastException e) {
            return false;
        }
        if (output != nullptr) {
            setClipData(ClipData.newRawUri("", output));
            addFlags(FLAG_GRANT_WRITE_URI_PERMISSION|FLAG_GRANT_READ_URI_PERMISSION);
            return true;
        }
    }

    return false;
}

std::string Intent::dockStateToString(int dock) {
    switch (dock) {
        case EXTRA_DOCK_STATE_HE_DESK:
            return "EXTRA_DOCK_STATE_HE_DESK";
        case EXTRA_DOCK_STATE_LE_DESK:
            return "EXTRA_DOCK_STATE_LE_DESK";
        case EXTRA_DOCK_STATE_CAR:
            return "EXTRA_DOCK_STATE_CAR";
        case EXTRA_DOCK_STATE_DESK:
            return "EXTRA_DOCK_STATE_DESK";
        case EXTRA_DOCK_STATE_UNDOCKED:
            return "EXTRA_DOCK_STATE_UNDOCKED";
        default:
            return Integer.toString(dock);
    }
}

private static ClipData.Item makeClipItem(ArrayList<Uri> streams, ArrayList<CharSequence> texts,
        ArrayList<String> htmlTexts, int which) {
    Uri uri = streams != null ? streams.get(which) : null;
    CharSequence text = texts != null ? texts.get(which) : null;
    std::string htmlText = htmlTexts != null ? htmlTexts.get(which) : null;
    return new ClipData.Item(text, htmlText, null, uri);
}
#endif

bool Intent::isDocument() const{
    return (mFlags & FLAG_ACTIVITY_NEW_DOCUMENT) == FLAG_ACTIVITY_NEW_DOCUMENT;
}
};
