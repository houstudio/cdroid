/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#ifndef __XML_PULLPARSER_H__
#define __XML_PULLPARSER_H__
#include <string>
#include <queue>
#include <memory>
#include <core/attributeset.h>
namespace cdroid{
class XmlPullParser:public AttributeSet{
public:
    enum EventType {
        BAD_DOCUMENT,
        START_DOCUMENT,
        END_DOCUMENT,

        START_TAG,
        END_TAG,
        TEXT,
        COMMENT
    };
private:
    class AttrParser;
    struct Private* mData;
    XmlPullParser();
public:
    XmlPullParser(Context*ctx,const std::string&resid);
    XmlPullParser(Context*,std::unique_ptr<std::istream>);
    ~XmlPullParser()override;
    int getDepth()const;
    std::string getName()const;
    std::string getText()const;
    std::string getPositionDescription()const;
    int getEventType()const;
    int getLineNumber()const;
    int getColumnNumber()const;
    int next();
    operator bool()const;
};
}
#endif /*__XML_PULLPARSER_H__*/
