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
#ifndef PUPPETMASTERGUARDIAN_H
#define PUPPETMASTERGUARDIAN_H

#include "puppetMasterFR.h"

class securityCamera;

/**
 * @brief Drives a guardian drone for a security camera.
 *
 * When the guardian itself sees the player within difficulty::cameraSight, it fights: it hurts the player
 * when next to them, shoots along a clear row or column, and chases them otherwise. When its
 * camera reports a new sighting, it walks there to check. It never leaves the camera's leash,
 * and when there is nothing to do it patrols around the camera.
 */
class puppetMasterGuardian : public puppetMasterFR
{
    friend class gameSerializer;

public:
    static constexpr int meleeDamage = 5;

    void guard(std::shared_ptr<securityCamera> cam);
    std::shared_ptr<securityCamera> getCamera() const { return this->camera.lock(); }
    void onAttach(std::shared_ptr<bElem> body) override;
    bool drive(std::shared_ptr<bElem> body) override;

private:
    bool fight(std::shared_ptr<bElem> body, std::shared_ptr<bElem> prey);
    std::weak_ptr<securityCamera> camera;
    std::shared_ptr<bElem> gun;
    coords home = NOCOORDS;     ///< the centre of the leash when the camera is gone
    coords target = NOCOORDS;   ///< where we are going to check
    unsigned int handledAlert = 0;
};

#endif // PUPPETMASTERGUARDIAN_H
