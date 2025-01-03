#include <Coords.h>
#include <cmath>

namespace myUtility
{
    /**
     * @brief Constructs a Coords object with specified x and y values.
     *
     * This constructor initializes the Coords object with the provided x and y coordinates.
     *
     * @param x_val The initial x-coordinate.
     * @param y_val The initial y-coordinate.
     */
    Coords::Coords(int x_val, int y_val) : x(x_val), y(y_val), flat(true)
    {

    }

/**
   * @brief Constructs a Coords object with specified x and y values.
   *
   * This constructor initializes the Coords object with the provided x and y coordinates.
   *
   * @param x_val The initial x-coordinate.
   * @param y_val The initial y-coordinate.
   * @param z_val The initial z-coordinate.
   */
    Coords::Coords(int x_val, int y_val, int z_val) : x(x_val), y(y_val), z(z_val), flat(false)
    {

    }

/**
 * @brief Copy constructor for Coords.
 *
 * This constructor creates a new Coords object that is a copy of the provided Coords instance.
 *
 * @param other The Coords object to be copied.
 */
    Coords::Coords(const Coords &other) : x(other.x), y(other.y)
    {
    }

/**
 * @brief Assignment operator for Coords.
 *
 * Assigns the values of another Coords object to this Coords instance.
 *
 * @param other The Coords object whose values will be assigned.
 * @return A reference to the assigned Coords object.
 */
    Coords &Coords::operator=(const Coords &other)
    {
        if (this != &other) {
            x = other.getX();
            y = other.getY();
        }
        return *this;
    }

/**
 * @brief Equality operator for Coords.
 *
 * Compares two Coords objects for equality, based on their x and y coordinates.
 *
 * @param other The Coords object to compare with.
 * @return True if both Coords objects have the same coordinates, otherwise false.
 */
    bool Coords::operator==(const Coords &other) const
    {
        return (x == other.getX() && y == other.getY());
    }

/**
 * @brief Adds the coordinates of two Coords objects.
 *
 * This operator returns a new Coords object, where the x and y coordinates
 * are the sum of the corresponding coordinates from the two operands.
 *
 * @param other The Coords object to add.
 * @return A new Coords object with the summed coordinates.
 */
    Coords Coords::operator+(const Coords &other) const
    {
        return Coords(x + other.getX(), y + other.getY());
    }

/**
 * @brief Subtracts the coordinates of one Coords object from another.
 *
 * This operator returns a new Coords object, where the x and y coordinates
 * are the difference between the corresponding coordinates of the two operands.
 *
 * @param other The Coords object to subtract.
 * @return A new Coords object with the subtracted coordinates.
 */
    Coords Coords::operator-(const Coords &other) const
    {
        return Coords(x - other.getX(), y - other.getY());
    }

/**
 * @brief Multiplies the coordinates of a Coords object by an integer scalar.
 *
 * This operator returns a new Coords object, where both x and y coordinates
 * are multiplied by the given scalar.
 *
 * @param scalar The integer by which to multiply the coordinates.
 * @return A new Coords object with the scaled coordinates.
 */
    Coords Coords::operator*(int scalar) const
    {
        return Coords(x * scalar, y * scalar);
    }

/**
 * @brief Multiplies the coordinates of two Coords objects element-wise.
 *
 * This operator returns a new Coords object where the x coordinate is the product of the x coordinates
 * of the two operands, and the y coordinate is the product of the y coordinates.
 *
 * @param other The Coords object to multiply with.
 * @return A new Coords object with the element-wise multiplied coordinates.
 */
    Coords Coords::operator*(const Coords &other) const
    {
        return Coords(x * other.getX(), y * other.getY());
    }


/**
 * @brief Divides the coordinates of a Coords object by an integer scalar.
 *
 * This operator returns a new Coords object, where both x and y coordinates
 * are divided by the given scalar.
 *
 * @param scalar The integer by which to divide the coordinates.
 * @return A new Coords object with the scaled down coordinates.
 */
    Coords Coords::operator/(int scalar) const
    {
        return Coords(x / scalar, y / scalar);
    }

/**
 * @brief Divides the coordinates of two Coords objects element-wise.
 *
 * This operator returns a new Coords object where the x coordinate is the product of the x coordinates
 * of the two operands, and the y coordinate is the product of the y coordinates.
 *
 * @param other The Coords object to divide with.
 * @return A new Coords object with the element-wise divided coordinates.
 */
    Coords Coords::operator/(const Coords &other) const
    {
        return Coords(x / other.getX(), y / other.getY());
    }


/**
 * @brief Adds a scalar value to both coordinates of a Coords object.
 *
 * This operator returns a new Coords object where both the x and y coordinates are increased
 * by the specified scalar value.
 *
 * @param scalar The value to add to the coordinates.
 * @return A new Coords object with the increased coordinates.
 */
    Coords Coords::operator+(int scalar) const
    {
        return Coords(x + scalar, y + scalar);
    }

/**
 * @brief Subtracts a scalar value from both coordinates of a Coords object.
 *
 * This operator returns a new Coords object where both the x and y coordinates are decreased
 * by the specified scalar value.
 *
 * @param scalar The value to subtract from the coordinates.
 * @return A new Coords object with the decreased coordinates.
 */
    Coords Coords::operator-(int scalar) const
    {
        return Coords(x - scalar, y - scalar);
    }

/**
 * @brief Calculates the Euclidean distance to another Coords object.
 *
 * This method computes the distance between this Coords object and another,
 * using the Euclidean distance formula.
 *
 * @param other The Coords object to which the distance will be calculated.
 * @return The Euclidean distance as a double.
 */
    double Coords::distance(const Coords &other) const
    {
        int dx = x - other.getX();
        int dy = y - other.getY();
        return std::sqrt(dx * dx + dy * dy);
    }

/**
 * @brief Gets the x-coordinate.
 *
 * @return The x-coordinate as an integer.
 */
    int Coords::getX() const
    {
        return x;
    }

/**
 * @brief Gets the y-coordinate.
 *
 * @return The y-coordinate as an integer.
 */
    int Coords::getY() const
    {
        return y;
    }

/**
 * @brief Sets the x-coordinate.
 *
 * @param x_val The new value for the x-coordinate.
 */
    Coords &Coords::setX(int x_val)
    {
        x = x_val;
        return *this;
    }


/**
 * @brief Sets the y-coordinate.
 *
 * @param y_val The new value for the y-coordinate.
 */
    Coords &Coords::setY(int y_val)
    {
        y = y_val;
        return *this;
    }

/**
 * @brief Modulo operation with a scalar.
 *
 * This operator calculates the modulo of both x and y coordinates by the given scalar.
 *
 * @param scalar The scalar value to use for the modulo operation.
 * @return A new Coords object where x and y are the result of x % scalar and y % scalar.
 */
    Coords Coords::operator%(int scalar) const
    {
        return Coords(x % scalar, y % scalar);
    }

/**
 * @brief Modulo operation with another Coords object.
 *
 * This operator calculates the modulo of x and y coordinates with the corresponding values of another Coords object.
 *
 * @param other Another Coords object to use for the modulo operation.
 * @return A new Coords object where x and y are the result of x % other.x and y % other.y.
 */
    Coords Coords::operator%(const Coords &other) const
    {
        return Coords(x % other.getX(), y % other.getY());
    }

/**
 * @brief Temporary operator for the transition from coords
 */

    Coords &Coords::operator=(const coords &other)
    {
        this->x = other.x;
        this->y = other.y;
        return *this;
    }

/**
 * @brief Temporary operator for the transition from coords
 */

    bool Coords::operator==(const coords &other)
    {
        return this->x == other.x && this->y == other.y;
    }

    int Coords::sum2d() const
    {
        return x + y;
    }
    int Coords::sum3d() const
    {
        return x + y+z;
    }

    Coords::Coords(coords coords1):x(coords1.x),y(coords1.y),flat(true)
    {

    }

    Coords Coords::dir2coords(dir::direction dir)
    {
        const std::array<Coords, 5> directionToCoordsMap = {
                Coords(0, -1),  // UP
                Coords(-1, 0),  // LEFT
                Coords(0, 1),   // DOWN
                Coords(1, 0),    // RIGHT
                Coords(0,0),      //NOP
        };
        return directionToCoordsMap[static_cast<int>(dir)];
    }


};
