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
#ifndef __GRAPH_CONTEXT_H__
#define __GRAPH_CONTEXT_H__
#include <cdtypes.h>
#include <cdgraph.h>
#include <cmath>
#include <cairomm/region.h>
#include <cairomm/context.h>
#include <cairomm/fontoptions.h>
#include <core/graphdevice.h>
#include <core/color.h>
namespace cdroid{

class Canvas:public Cairo::Context{
protected:
    void*mHandle;
    friend class Window;
    friend class GraphDevice;
    friend class WindowManager;
public:
    Canvas(const Cairo::RefPtr<Cairo::Surface>&target);
    Canvas(unsigned int width,unsigned int height);
    ~Canvas();
    void*getHandler()const;
    void get_text_size(const std::string&txt,int*w,int*h); 
    void draw_text(const Rect&rect,const std::string&text,int text_alignment=0);
    void set_color(uint8_t r,uint8_t g, uint8_t b,uint8_t a=255);
    void set_color(uint32_t color);
    void rectangle(int x,int y,int w,int h);
    void rectangle(const Rect &r);
    void draw_image(const Cairo::RefPtr<Cairo::ImageSurface>&img,const Rect& dst,const Rect* src);
    void dump2png(const std::string& fname);
};
extern void DumpRegion(const std::string&label,const Cairo::RefPtr<Cairo::Region>&rgn);
}//namspace
#endif

