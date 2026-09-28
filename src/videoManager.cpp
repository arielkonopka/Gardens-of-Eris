#include "videoManager.h"

// Initialize static members of ShaderInfo
int videoManager::ShaderInfo::lastId = 5;
std::mutex videoManager::ShaderInfo::idMutex;

/**
 * @brief Constructor for ShaderInfo.
 *
 * In the spirit of Eris, we initiate the shader with a touch of chaos.
 *
 * @param fname The filename of the shader.
 */
videoManager::ShaderInfo::ShaderInfo(const std::string &vfname, const std::string &pfname)
    : psfilename(pfname)
    , vsfilename(vfname)
{
    std::lock_guard<std::mutex> lock(idMutex);
    id = lastId++;
}

/**
 * @brief Retrieves the singleton instance of videoManager.
 *
 * Ensures only one instance exists amidst the disorder. If not initialized yet,
 * the chaos of initialization ensues.
 *
 * @return The singleton instance of videoManager.
 */
videoManager &videoManager::getInstance()
{
    static videoManager instance;
    if (!instance.initialized) {
        if (!instance.initialize()) {
            std::cerr << "Failed to initialize video system!" << std::endl;
            throw std::runtime_error("Failed to initialize videoManager");
        }
    }

    return instance;
}

/**
 * @brief Private constructor for Singleton pattern.
 *
 * Concealed to maintain singularity in the midst of chaos.
 */
videoManager::videoManager()
    : initialized(false)
{}

/**
 * @brief Destructor that cleans up video resources.
 *
 * Returns all organized structures back to primordial chaos.
 */
videoManager::~videoManager()
{
    shutdown();
}

/**
 * @brief Initializes Allegro and video resources.
 *
 * Sets up the video system, taming the initial chaos for a moment.
 *
 * @return true if initialization was successful, false otherwise.
 */
bool videoManager::initialize()
{
    if (!al_init()) {
        std::cerr << "Failed to initialize Allegro!\n";
        return false;
    }

    if (!al_init_image_addon()) {
        std::cerr << "Failed to initialize image addon!\n";
        return false;
    }
    ALLEGRO_MONITOR_INFO info;
    //  al_set_new_display_flags(ALLEGRO_FULLSCREEN);
    al_set_new_display_flags(ALLEGRO_OPENGL | ALLEGRO_OPENGL_3_0
                             | ALLEGRO_PROGRAMMABLE_PIPELINE); //ma| ALLEGRO_FULLSCREEN);
    al_inhibit_screensaver(true);
    al_set_new_display_option(ALLEGRO_VSYNC, 0, ALLEGRO_REQUIRE);
    al_get_monitor_info(0, &info);
    this->display.reset(al_create_display(info.x2 - info.x1, info.y2 - info.y1));
    if (!display) {
        std::cerr << "Failed to create display!\n";
        return false;
    }
    al_hide_mouse_cursor(this->display.get());
    al_init_primitives_addon();
    initialized = true; // Set initialized flag to true after successful initialization
    return true;
}

/**
 * @brief Shuts down and cleans up video resources.
 *
 * Dissolves the order back into entropy.
 */
void videoManager::shutdown()
{
    if (!initialized)
        return;
    al_use_shader(nullptr);
    destroyAllShaders();
    al_shutdown_primitives_addon();
    al_shutdown_image_addon();

    display.reset();

    initialized = false;
}

/**
 * @brief Sets up a shader from a file.
 *
 * Introduces a new element of discord by creating a shader.
 *
 * @param fname The filename of the shader.
 * @return The unique ID of the shader, or -1 on failure.
 */
int videoManager::setupShader(const std::string &vxfname, const std::string &pxfname)
{
    ShaderInfo newShader(vxfname, pxfname);
    newShader.shader.reset(al_create_shader(ALLEGRO_SHADER_GLSL));
    std::cerr << "Creating shader no:" << newShader.id << "\n";
    if (!newShader.shader) {
        std::cerr << "Failed to create shader for files: " << vxfname << " " << pxfname
                  << std::endl;
        return -1; // Indicate failure
    }
    auto *shader = newShader.shader.get();
    if (!al_attach_shader_source_file(shader, ALLEGRO_VERTEX_SHADER, vxfname.c_str())) {
        std::cerr << "Failed to attach vertex shader source from file: " << vxfname << std::endl;
        return -1;
    }
    if (!al_attach_shader_source_file(shader, ALLEGRO_PIXEL_SHADER, pxfname.c_str())) {
        std::cerr << "Failed to attach pixel shader source from file: " << pxfname << std::endl;
        return -1;
    }
    if (!al_build_shader(shader)) {
        std::cerr << "Failed to create shader for files: " << vxfname << " " << pxfname
                  << std::endl;
        std::cerr << "Shader Log: " << al_get_shader_log(shader) << std::endl;
        return -1;
    }
    const int id = newShader.id;
    shaders.emplace(id, std::move(newShader));
    return id;
}

/**
 * @brief Retrieves a shader by its unique ID.
 *
 * Navigates the disorder to find the elusive shader.
 *
 * @param id The unique ID of the shader.
 * @return A pointer to the ALLEGRO_SHADER, or nullptr if not found.
 */
ALLEGRO_SHADER *videoManager::getShader(int id)
{
    auto it = shaders.find(id);
    if (it != shaders.end()) {
        return it->second.shader.get();
    }
    return nullptr;
}

/**
 * @brief Destroys a shader by its unique ID.
 *
 * Returns the shader to the embrace of entropy.
 *
 * @param id The unique ID of the shader to destroy.
 * @return true if the shader was successfully destroyed, false if not found.
 */
bool videoManager::destroyShader(int id)
{
    auto it = shaders.find(id);
    if (it != shaders.end()) {
        shaders.erase(it);
        return true;
    }
    return false; // Shader with the given ID was not found
}

/**
 * @brief Destroys all shaders.
 *
 * Embraces entropy by eliminating all structured shaders.
 */
void videoManager::destroyAllShaders()
{
    shaders.clear();
}

/**
 * @brief Gets the backbuffer bitmap of the display.
 *
 * Retrieves the canvas upon which chaos is rendered.
 *
 * @return A pointer to the display's backbuffer bitmap.
 */
ALLEGRO_BITMAP *videoManager::getDisplayBitmap()
{
    if (display) {
        return al_get_backbuffer(display.get());
    }
    return nullptr;
}

/**
 * @brief Gets the size of the screen.
 *
 * Measures the dimensions of our realm of disorder.
 *
 * @return The coordinates representing screen width and height.
 */
coords videoManager::getScreenSize()
{
    return coords(al_get_display_width(display.get()), al_get_display_height(display.get()));
}

/**
 * @brief Gets the size of the game field.
 *
 * Determines the battleground where chaos and order collide.
 *
 * @return The coordinates representing game field width and height.
 */
coords videoManager::getGameFieldSize()
{
    coords tileNum = (this->getScreenSize()
                      / coords(configManager::getInstance()->getConfig()->tileWidth,
                               configManager::getInstance()->getConfig()->tileHeight));
    tileNum = tileNum - coords(2, 5); // how many columns and rows to be removed from the game field
    coords gfSize(tileNum
                  * coords(configManager::getInstance()->getConfig()->tileWidth,
                           configManager::getInstance()->getConfig()->tileHeight));
    return gfSize;
}

/**
 * @brief Gets the size of the HUD.
 *
 * Defines the interface between the player and the unfolding chaos.
 *
 * @return The coordinates representing HUD width and height.
 */
coords videoManager::getHudSize()
{
    return coords(getGameFieldSize().x, configManager::getInstance()->getConfig()->tileHeight * 5);
}

ALLEGRO_DISPLAY *videoManager::getCurrentDisplay() const
{
    return this->display.get();
}
