/*
 * Copyright (c) 2026, Ariel Konopka
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


#ifndef ISOFIELDVIEW_H
#define ISOFIELDVIEW_H
#include "allegroHandles.h"
#include "fieldView.h"
#include "fogLayer.h"
#include "isoMesh.h"
#include "modelLibrary.h"
#include <allegro5/allegro_primitives.h>
#include <array>
#include <string>
#include <vector>

/**
 * The game field in 3D, seen by an isometric camera: walls are blocks, the floor is a plane, and
 * creatures and things stand up as pictures. Every frame it builds the mesh of the cells the
 * camera can see (goe::iso::meshBuilder), straight from the skin's sprite sheet, and draws it in
 * one call with a depth buffer and the same fog as the 2D view. Elements the skin gives a 3D
 * model ("Model" in skins.json) are drawn as that model instead, one call each.
 */
class chamber;

class isoFieldView : public fieldView
{
public:
    /// how many screen pixels one cell spans, as a part of the tile size
    static constexpr float zoom = 1.0f;
    /// the camera looks down at 35.26 degrees, true isometric
    static constexpr float pitch = 0.6154797f;

    /// needs the display; fieldPx is the field's size on screen
    isoFieldView(coords fieldPx, coords tileSize, int spacing, const std::string &fogBitmap);
    void draw(const fieldFrame &frame) override;
    /// false when the depth buffer or the shader could not be made
    bool ready() const;
    /// cells drawn around the centre, each way
    int reach() const { return this->radius; }

private:
    /// a model standing on the field this frame
    struct placedModel {
        const modelLibrary::model *model;
        float x, y;
        int quarters;
        /// below 1 for a wall cut low so the avatar shows
        float squash;
        std::array<float, 3> tint;
        /// the lid over a wall model's cell, instead of the model itself
        bool lid = false;
    };
    /// puts the element's model on the field; false when it has none
    bool addModel(const videoElement::videoElementDef &ve,
                  int subType,
                  int facing,
                  coords at,
                  float x,
                  float y,
                  bool cut);
    /// whether the cell holds a block, which a wall model joins without a side in between
    bool joins(coords at) const;
    void drawModels();
    /// the cells of the window starting at origin that the camera sees, as triangles
    void buildMesh(const fieldFrame &frame, coords origin, float cx, float cy);
    void useCamera(float cx, float cy);
    void setFogUniforms(coords origin);
    coords tileSize;
    int spacing;
    coords fieldPx;
    int radius;
    int shaderId = -1;
    goe::bitmapHandle field;
    fogLayer fog;
    goe::iso::meshBuilder mesh;
    std::vector<ALLEGRO_VERTEX> vertices;
    modelLibrary models;
    std::vector<placedModel> placed;
    /// the board drawn this frame, where wall models look at their neighbours
    std::shared_ptr<chamber> shownBoard;
    /// the colour under the floor's see-through pictures, as on the 2D board
    std::array<float, 3> boardColour{0, 0, 0};
};

#endif // ISOFIELDVIEW_H
