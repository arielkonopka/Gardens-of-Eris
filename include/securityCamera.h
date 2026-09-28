/*
 * Copyright (c) 2023, Ariel Konopka
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
#ifndef SECURITYCAMERA_H
#define SECURITYCAMERA_H

#include "bElem.h"

/**
 * @brief A fixed camera that watches for the player and calls its guardian drones.
 *
 * On its first tick it spawns difficulty::guardianCount patrolling drones next to itself, each driven by a
 * puppetMasterGuardian linked to this camera. Whenever it sees the active player within
 * difficulty::cameraSight, with nothing solid in between, it records where, and the guardians go there.
 * Guardians never leave the circle of radius leash around the camera.
 */
class securityCamera : public bElem
{
    friend class gameSerializer;

public:
    static constexpr int leash = 55;

    securityCamera() = default;
    ~securityCamera() override = default;
    bool additionalProvisioning(int subtype) override;
    int getType() const override;
    bool mechanics() override;

    /// where the player was last seen; NOCOORDS until the camera has seen them
    coords getAlertPosition() const { return this->alertAt; }
    /// grows by one every time the camera sees the player, so guardians can tell a new sighting
    unsigned int getAlertNumber() const { return this->alertNumber; }
    /// true when nothing solid (drones aside) lies on the straight line between the two cells
    static bool lineOfSight(std::shared_ptr<chamber> board, coords from, coords to);

private:
    void spawnGuardians();
    bool guardiansSpawned = false;
    coords alertAt = NOCOORDS;
    unsigned int alertNumber = 0;
};

#endif // SECURITYCAMERA_H
