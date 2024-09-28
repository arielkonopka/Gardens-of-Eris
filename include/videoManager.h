#ifndef VIDEOMANAGER_H
#define VIDEOMANAGER_H

#include <allegro5/allegro.h>
#include <mutex>
#include <allegro5/allegro_image.h>
#include <iostream>
#include <map>

#include "commons.h"
#include "configManager.h"

/**
 * @class videoManager
 * @brief Singleton class for managing video resources and display initialization using Allegro 5.
 *
 * This class ensures that there is only one instance of videoManager in the game,
 * and provides controlled access to it. The object is always initialized on
 * `getInstance()` to ensure video resources are ready.
 */
class videoManager {
public:
    /**
     * @brief Retrieves the singleton instance of videoManager.
     *
     * This method ensures that only one instance of videoManager exists and provides
     * global access to it. Additionally, it ensures that the instance is always initialized.
     *
     * @return The singleton instance of videoManager.
     */
    static videoManager& getInstance();

    /**
     * @brief Shuts down and cleans up video resources.
     */
    void shutdown();

    /**
     * @brief Gets the backbuffer bitmap of the display.
     *
     * This method returns the bitmap associated with the display's backbuffer.
     * Other classes can use this bitmap for drawing.
     *
     * @return A pointer to the display's backbuffer bitmap.
     */
    ALLEGRO_BITMAP* getDisplayBitmap();

    coords getScreenSize();
    coords getGameFieldSize();
    coords getHudSize();

    // Delete copy constructor and assignment operator to prevent copying
    videoManager(const videoManager&) = delete;
    videoManager& operator=(const videoManager&) = delete;

    /**
     * @brief Sets up a shader from a file.
     *
     * @param fname The filename of the shader.
     * @return The unique ID of the shader, or -1 on failure.
     */
    int setupShader(const std::string& vxfname,const std::string& pxfname);

    /**
     * @brief Retrieves a shader by its unique ID.
     *
     * @param id The unique ID of the shader.
     * @return A pointer to the ALLEGRO_SHADER, or nullptr if not found.
     */
    ALLEGRO_SHADER* getShader(int id);
    /**
     * @brief Destroys a shader by its unique ID.
     *
     * @param id The unique ID of the shader to destroy.
     * @return true if the shader was successfully destroyed, false if not found.
     */
    bool destroyShader(int id);

    /**
     * @brief Destroys all shaders.
     */
    void destroyAllShaders();
    ALLEGRO_DISPLAY*getCurrentDisplay() const;
private:
    /**
     * @brief Class for storing information about a shader.
     */
    class ShaderInfo {
    public:
        /**
         * @brief Constructor for ShaderInfo.
         *
         * Initializes the shader information and assigns a unique ID.
         *
         * @param fname The filename of the shader.
         */
        ShaderInfo(const std::string& vfname,const std::string& pfname);

        /**
         * @brief Destructor for ShaderInfo.
         *
         * Destroys the shader if it has been initialized.
         */
         ~ShaderInfo();
        // Member variables
        std::string psfilename;       /**< The filename of the shader */
        std::string vsfilename;       /**< The filename of the shader */
        int id;                     /**< The unique ID of the shader */
        bool initialized;           /**< Flag indicating if the shader has been initialized */
        ALLEGRO_SHADER* shader;     /**< Pointer to the shader object */
        // Implement move constructor and move assignment operator
        ShaderInfo(ShaderInfo&& other) noexcept;
        ShaderInfo& operator=(ShaderInfo&& other) noexcept;
        // Existing constructors and methods...

        /**
         * @brief Equality operator for ShaderInfo.
         *
         * In the dance of discord, determines if two shaders share the same chaotic essence by comparing their IDs.
         *
         * @param other The ShaderInfo to compare with.
         * @return True if both shaders have the same id, false otherwise.
         */
        bool operator==(const ShaderInfo& other) const {
            return id == other.id;
        }



    private:
        static int lastId;          /**< Static member to generate unique IDs */
        static std::mutex idMutex;  /**< Mutex to protect ID generation */

    };

    // Other private members of videoManager...
    std::map<int,ShaderInfo> shaders{}; /**< Allegro shader object vector */
    ALLEGRO_DISPLAY* display;       /**< Allegro display object for the game window */
    bool initialized;               /**< Flag to track if the videoManager is initialized */

    /**
     * @brief Initializes Allegro and the video resources.
     *
     * This method sets up Allegro and its necessary addons, including
     * the image addon, and creates the game window.
     *
     * @return true if initialization was successful, false otherwise.
     */
    bool initialize();

    /**
     * @brief Private constructor for Singleton pattern.
     *
     * The constructor is private to ensure that the class can only be instantiated
     * through the getInstance() method.
     */
    videoManager();

    /**
     * @brief Destructor for cleaning up resources.
     */
    ~videoManager();

    static std::once_flag initFlag; /**< Ensures the Singleton is initialized only once */
    int scrWidth = -1;
    int scrHeight = -1;
};

#endif // VIDEOMANAGER_H
