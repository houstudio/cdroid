/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __ABILITY_BASE_URI_H__
#define __ABILITY_BASE_URI_H__

#include <string>
#include <vector>
#include "parcel.h"

namespace cdroid {
class Uri/*: public Parcelable*/{
public:
    explicit Uri(const std::string& uriString);
    ~Uri();

    /**
     * Get the Scheme part.
     *
     * @return the scheme string.
     */
    std::string getScheme();

    /**
     * Get the Ssp part.
     *
     * @return the SchemeSpecificPart string.
     */
    std::string getSchemeSpecificPart();

    /**
     * Get the GetAuthority part.
     *
     * @return the authority string.
     */
    std::string getAuthority();

    /**
     * Get the Host part.
     *
     * @return the host string.
     */
    std::string getHost();

    /**
     * Get the Port part.
     *
     * @return the port number.
     */
    int getPort();

    /**
     * Get the User part.
     *
     * @return the user string.
     */
    std::string getUserInfo();

    /**
     * Get the Query part.
     *
     * @return the query string.
     */
    std::string getQuery();

    /**
     * Get the Path part.
     *
     * @return the path string.
     */
    std::string getPath();

    /**
     * Get the path segments.
     *
     * @param the path segments of Uri.
     */
    void getPathSegments(std::vector<std::string>& segments);

    /**
     * Get the Fragment part.
     *
     * @return the fragment string.
     */
    std::string getFragment();

    /**
     * Returns true if this URI is hierarchical like "http://www.example.com".
     * Absolute URIs are hierarchical if the scheme-specific part starts with a '/'.
     * Relative URIs are always hierarchical.
     *
     * @return true if this URI is hierarchical, false if it's opaque.
     */
    bool isHierarchical();

    /**
     * Returns true if this URI is absolute, i.e.&nbsp;if it contains an explicit scheme.
     *
     * @return true if this URI is absolute, false if it's relative.
     */
    bool isAbsolute();

    /**
     * Returns true if this URI is relative, i.e.&nbsp;if it doesn't contain an explicit scheme.
     *
     * @return true if this URI is relative, false if it's absolute.
     */
    bool isRelative();

    /**
     * Check whether the other is the same as this.
     *
     * @return true if the same string.
     */
    bool equals(const Uri& other) const;

    /**
     * Compare to other uri.
     *
     * @return the string compare result.
     */
    int compareTo(const Uri& other) const;

    /**
     * Convert to a string object.
     *
     * @return a string object.
     */
    std::string toString() const;

    /**
     * override the == method.
     *
     * @return true if the same content, false not the same content.
     */
    bool operator==(const Uri& other) const;

    /**
     * Override Parcelable' interface.
     *
     * @return true if parcel write success, false write fail.
     */
    //virtual bool marshalling(Parcel& parcel) const override;

    /**
     * Support the Ummarshlling method for construct object by  Parcel read.
     *
     * @return the uri object address.
     */
    //static Uri* Unmarshalling(Parcel& parcel);
    static std::string decode(const std::string&);
private:
    bool checkScheme();
    std::string parseScheme();
    std::string parseSsp();
    std::string parseAuthority();
    std::string parseUserInfo();
    std::string parseHost();
    int parsePort();
    std::string parsePath(size_t ssi);
    std::string parsePath();
    std::string parseQuery();
    std::string parseFragment();

    /**
    * Finds the first ':'.
    *
    * @return the pos of the ':', string::npos if none found.
    */
    size_t findSchemeSeparator();

    /**
     * Finds the first '#'.
     *
     * @return the pos of the '#', string::npos if none found.
     */
    size_t findFragmentSeparator();

    std::string uriString_;
    std::string scheme_;
    std::string ssp_;
    std::string authority_;
    std::string host_;
    int port_;
    std::string userInfo_;
    std::string query_;
    std::string path_;
    std::string fragment_;
    size_t cachedSsi_;
    size_t cachedFsi_;
};
} // namespace cdroid
#endif // __ABILITY_BASE_URI_H__
