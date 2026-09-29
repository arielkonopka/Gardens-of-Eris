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

#ifndef TELEPORT_H
#define TELEPORT_H

#include "commons.h"
#include <mutex>
#include "videoElementDef.h"
#include "randomLevelGenerator.h"
#include "bElem.h"
class teleport : public bElem
{
    friend class gameSerializer;
public:
    using bElem::additionalProvisioning;

    int getType() const override;
    teleport()=default;
    ~teleport() override=default;
    bool interact(std::shared_ptr<bElem> who) override;
    virtual bool teleportIt(std::shared_ptr<bElem> who);
    oState disposeElement() override;
    bool createConnectionsWithinSubtype();
    bool additionalProvisioning(int value) override;
    bool stepOnElement(std::shared_ptr<bElem> step) override;
    bool mechanics() final;
    bool stepOnAction(bool step,std::shared_ptr<bElem> who) override;

    /**
     * @brief Defers teleporter registration on the current thread until the batch ends.
     *
     * Levels are generated on a background thread while the game runs. Teleporters created
     * inside a batch are kept aside and added to the shared registry all at once when the batch
     * is destroyed, so pairing (especially global, cross-level pairing) only sees complete levels.
     */
    class registrationBatch
    {
    public:
        registrationBatch();
        ~registrationBatch();
        registrationBatch(const registrationBatch &) = delete;
        registrationBatch &operator=(const registrationBatch &) = delete;
    };
private:
    static std::vector<std::weak_ptr<teleport>> allTeleporters;
    static std::recursive_mutex registryMutex;
    static thread_local bool deferRegistration;
    static thread_local std::vector<std::weak_ptr<teleport>> pendingTeleporters;
    bool removeFromAllTeleporters();
    std::weak_ptr<teleport> theOtherEnd;
    std::vector<std::shared_ptr<teleport>> candidates;
    static bool firstReceiverRemoved;
};

#endif // TELEPORT_H
