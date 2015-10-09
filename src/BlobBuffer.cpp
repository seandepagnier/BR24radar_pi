/******************************************************************************
 *
 * Project:  OpenCPN Navico BR24 Radar Plugin
 * Purpose:  Radar Blob Buffer
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */

#include <wx/wx.h>
#include <wx/glcanvas.h>

#include "BlobBuffer.h"

BlobBuffer::BlobBuffer()
{
    count = size = 0;
    colors = NULL;
    triangles = NULL;
}

BlobBuffer::~BlobBuffer()
{
    delete [] colors;
    delete [] triangles;
}

void BlobBuffer::Push(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha,
                      float angle, float arc_width, float r_begin, float r_end)
{
    Blob blob;
    blob.c[0] = red, blob.c[1] = green, blob.c[2] = blue, blob.c[3] = alpha;

    float sa = sinf(angle), ca = cosf(angle);
    
    float xm1 = r_begin * ca;
    float ym1 = r_begin * sa;
    float xm2 = r_end * ca;
    float ym2 = r_end * sa;

    float arc_width_start2 = r_begin * arc_width;
    float arc_width_end2 =   r_end * arc_width;

    blob.q[0] = xm1 + arc_width_start2 * sa;
    blob.q[1] = ym1 - arc_width_start2 * ca;

    blob.q[2] = xm2 + arc_width_end2 * sa;
    blob.q[3] = ym2 - arc_width_end2 * ca;

    blob.q[4] = xm1 - arc_width_start2 * sa;
    blob.q[5] = ym1 + arc_width_start2 * ca;

    blob.q[6] = xm2 - arc_width_end2 * sa;
    blob.q[7] = ym2 + arc_width_end2 * ca;

    buffer.push_back(blob);
}

void BlobBuffer::DrawDC(wxDC &dc, wxPoint center, float scale)
{
    m_mutex.Lock();

    unsigned char *c = colors;
    wxColour colour(c[0], c[1], c[2], c[3]);
    dc.SetPen(colour);
    dc.SetBrush(colour);

    for(int i=0; i<count; i++) {
        unsigned char *c = colors + i*3*4;
        wxPoint points[3];
        for(int j = 0; j<3; j++) {
            points[j].x = triangles[(i*3+j)*2+0]*scale;
            points[j].y = triangles[(i*3+j)*2+1]*scale;
        }
        dc.DrawPolygon(3, points, center.x, center.y);
    }

    m_mutex.Unlock();
}

void BlobBuffer::DrawGL()
{
    m_mutex.Lock();

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
    glVertexPointer(2, GL_FLOAT, 0, triangles);
    glDrawArrays(GL_LINES, 0, 3*count);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    m_mutex.Unlock();
}

void BlobBuffer::Finalize()
{
    // could instead use page flipping with two buffers to minimize time mutex is locked
    m_mutex.Lock();

    count = 2*buffer.size();
    if(count > size) {
        delete [] colors;
        delete [] triangles;

        colors = new unsigned char[3*4*count];
        triangles = new float[3*2*count];

        size = count;
    }

    int ci = 0, ti = 0;
    for(std::list<Blob>::iterator it = buffer.begin(); it != buffer.end(); it++) {
        for(int k = 0; k<3; k++)
            memcpy(colors + ci, it->c, 4), ci+=4;
        memcpy(triangles + ti, it->q, 6*sizeof(float)), ti += 6;
        
        for(int k = 0; k<3; k++)
            memcpy(colors + ci, it->c, 4), ci+=4;
        memcpy(triangles + ti, it->q+2, 6*sizeof(float)), ti += 6;
    }

    m_mutex.Unlock();

    buffer.clear();
}
