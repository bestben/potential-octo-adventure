/**
 * @file
 * @copyright Copyright 2014 zargot. GNU General Public License.
 * @see <http://www.gnu.org/licenses/>
 *
 * Based on
 * OpenSimplexNoise (Simplectic) Noise in Java.
 * by Kurt Spencer
 * v1.1 (October 5, 2014)
 */

#pragma once

#include <array>

class OpenSimplexNoise {
public:
    OpenSimplexNoise(long seed = 0);
    double value(double x, double y);
    double value(double x, double y, double z);
    double value(double x, double y, double z, double w);

private:
	static const double STRETCH_CONSTANT_2D;
	static const double SQUISH_CONSTANT_2D;
	static const double STRETCH_CONSTANT_3D;
	static const double SQUISH_CONSTANT_3D;
	static const double STRETCH_CONSTANT_4D;
	static const double SQUISH_CONSTANT_4D;
	static const double NORM_CONSTANT_2D;
	static const double NORM_CONSTANT_3D;
	static const double NORM_CONSTANT_4D;
    static const std::array<char, 16> gradients2D;
    static const std::array<char, 72> gradients3D;
    static const std::array<char, 256> gradients4D;
	short perm[256];
	short permGradIndex3D[256];

    double extrapolate(int xsb, int ysb, double dx, double dy);
    double extrapolate(int xsb, int ysb, int zsb, double dx, double dy,
            double dz);
    double extrapolate(int xsb, int ysb, int zsb, int wsb,
            double dx, double dy, double dz, double dw);
    int fastFloor(double x);
};

