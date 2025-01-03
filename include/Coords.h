#ifndef INC_COORDS_H_
#define INC_COORDS_H_

/**
 * @class Coords
 * @brief This class is for representing 2D coordinates (x, y).
 *
 * Coords class has x, y values and provide operations like addition, subtraction,
 * multiplication, and distance calculation. Very useful for 2D point management.
 */

#include "commons.h"

namespace myUtility {

    class Coords {
    private:
        int x=-6502; ///< x coordinate value
        int y=-6502; ///< y coordinate value
        int z=-6502; ///< z coordonate if 3d
        bool flat=true;
    public:

        /**
         * @brief Default constructor.
         */
         Coords()=default;
        /**
         * @brief Constructor that initializes Coords with specific x and y values.
         * @param x_val Set x coordinate value (default is -6502).
         * @param y_val Set y coordinate value (default is -6502).
         */

        Coords(int x_val, int y_val);
        /**
                * @brief Constructor that initializes Coords with specific x and y values.
                * @param x_val Set x coordinate value (default is -6502).
                * @param y_val Set y coordinate value (default is -6502).
                * @param z_val set z coordinate value (default is -6502).
                */

        Coords(int x_val, int y_val, int z_val);
        /**
         * @brief Copy constructor.
         * @param other Coords object for copy.
         */
        Coords(const Coords &other);

        /**
         * @brief Assignment operator.
         * @param other Coords object for assign.
         * @return Return self object reference after assign.
         */
        Coords &operator=(const Coords &other);

        /**
         * @brief Check equality of two Coords objects.
         * @param other Coords object for compare.
         * @return Return true if equal, else false.
         */
        bool operator==(const Coords &other) const;

        /**
         * @brief Add two Coords objects.
         * @param other Coords object for add.
         * @return Return new Coords object with added coordinates.
         */
        Coords operator+(const Coords &other) const;

        /**
         * @brief Subtract two Coords objects.
         * @param other Coords object for subtract.
         * @return Return new Coords object with subtracted coordinates.
         */
        Coords operator-(const Coords &other) const;

        /**
         * @brief Multiply Coords by integer scalar.
         * @param scalar Integer for multiply.
         * @return Return new Coords object with multiplied coordinates.
         */
        Coords operator*(int scalar) const;

        /**
         * @brief Multiply two Coords objects (not standard math).
         * @param other Coords object for multiply.
         * @return Return new Coords object with element-wise multiplied coordinates.
         */
        Coords operator*(const Coords &other) const;

        /**
         * @brief Divide Coords by integer scalar.
         * @param scalar Integer for division.
         * @return Return new Coords object with divided coordinates.
         */
        Coords operator/(int scalar) const;

        /**
         * @brief Divide two Coords objects (not standard math).
         * @param other Coords object for division.
         * @return Return new Coords object with element-wise divided coordinates.
         */
        Coords operator/(const Coords &other) const;

        /**
         * @brief Modulo operation with a scalar.
         *
         * This operator calculates the modulo of both x and y coordinates by the given scalar.
         *
         * @param scalar The scalar value to use for modulo operation.
         * @return A new Coords object where x and y are the result of x % scalar and y % scalar.
         */
        Coords operator%(int scalar) const;

        /**
         * @brief Modulo operation with another Coords object.
         *
         * This operator calculates the modulo of x and y coordinates with the corresponding values of another Coords object.
         *
         * @param other Another Coords object to use for modulo operation.
         * @return A new Coords object where x and y are the result of x % other.x and y % other.y.
         */
        Coords operator%(const Coords& other) const;
        /**
         * @brief Add scalar value to Coords.
         * @param scalar Integer for add.
         * @return Return new Coords object with scalar added to x and y.
         */
        Coords operator+(int scalar) const;

        /**
         * @brief Subtract scalar value from Coords.
         * @param scalar Integer for subtract.
         * @return Return new Coords object with scalar subtracted from x and y.
         */
        Coords operator-(int scalar) const;

        /**
         * @brief temporary operator of assignment from coords
         */

        Coords &operator=(const coords &other);

        /**
         * @brief temporary operator of comparison with coords
         */
        bool operator==(const coords &other);



        /**
         * @brief Calculate distance to another Coords object.
         * @param other Coords object for distance calculation.
         * @return Return distance as double.
         */
        [[nodiscard]] double distance(const Coords &other) const;

        /**
         * @brief Get x coordinate value.
         * @return Return x coordinate.
         */
        [[nodiscard]]int getX() const;

        /**
         * @brief Get y coordinate value.
         * @return Return y coordinate.
         */
        [[nodiscard]]int getY() const;

        /**
         * @brief Set x coordinate value.
         * @param x_val New x coordinate value.
         */
        Coords& setX(int x_val);

        /**
         * @brief Set y coordinate value.
         * @param y_val New y coordinate value.
         */
        Coords& setY(int y_val);

        int sum2d() const;
        int sum3d() const;

        Coords(coords coords1);
        static Coords dir2coords(dir::direction dir);
    };

    constexpr Coords NOCOORDS=Coords();


}

#endif