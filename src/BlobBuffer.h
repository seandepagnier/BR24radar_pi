/******************************************************************************
 *
 * Project:  OpenCPN Navico BR24 Radar Plugin
 * Purpose:  Radar
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

#include <list>

struct Blob
{
    unsigned char c[4];
    float q[8];
};

class BlobBuffer {
public:
    BlobBuffer();
    ~BlobBuffer();

    void Push(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha,
              float angle, float arc_width, float r_begin, float r_end);
    void DrawDC(wxDC &dc);
    void DrawGL();
    void Finalize();

    wxMutex m_mutex;

private:

    int count;
    int size;
    unsigned char *colors;
    float *triangles;
    
    std::list<Blob> buffer;
};
