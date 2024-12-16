#ifndef EYECANDY_MANAGER_HPP
#define EYECANDY_MANAGER_HPP

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <vector>
#include <cstdint>
#include "videoManager.h"

/**
 * @class eyecandyManager
 * @brief Manages visual effects like shadows in the game using Allegro 5.
 *
 * The eyecandyManager class is responsible for rendering a pixel map,
 * which is then used to overlay shadows onto elements of the game's maze.
 * It utilizes a single pixel map computed based on a specific pattern.
 *
 * This class is implemented as a Singleton to ensure only one instance
 * manages the visual effects throughout the game.
 */
class eyecandyManager {
public:
    /**
     * @brief Gets the singleton instance of eyecandyManager.
     *
     * Ensures that only one instance of the class exists.
     * @return Reference to the singleton instance.
     */
    static eyecandyManager& getInstance();

    /**
     * @brief Renders the pixel map based on the computed pattern.
     *
     * Prepares the pixel map for use when drawing shadows.
     */
    void renderPixelMap();

    /**
     * @brief Draws shadows onto game elements.
     *
     * Uses the rendered pixel map to overlay shadows in the maze.
     * @param targetBitmap The Allegro bitmap where the shadows will be drawn.
     */
    void drawShadow(ALLEGRO_BITMAP* targetBitmap);

    // Delete copy constructor and assignment operator to prevent copies
    eyecandyManager(const eyecandyManager&) = delete;
    eyecandyManager& operator=(const eyecandyManager&) = delete;

private:
    /**
     * @brief Private constructor for Singleton pattern.
     *
     * Initializes resources needed for pixel map rendering.
     */
    eyecandyManager();

    /**
     * @brief Private destructor.
     *
     * Cleans up any allocated resources.
     */
    ~eyecandyManager();

    /**
     * @brief Pixel map used for rendering shadows.
     *
     * Stores pixel data computed from the pattern.
     */
    ALLEGRO_BITMAP* pixelMap;

    /**
     * @brief Computes the pattern for the pixel map.
     *
     * Performs calculations needed to generate the appropriate pixel map pattern
     * based on the game's logic.
     */
    void computePattern();
};

#endif // EYECANDY_MANAGER_HPP
