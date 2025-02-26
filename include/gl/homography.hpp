#ifndef HOMOGRAPHY_HPP
#define HOMOGRAPHY_HPP

#include <array>
#include <glm/glm.hpp>
#include <stdexcept>
#include <cmath>

// Computes the homography H that maps 4 source points to 4 destination points.
// The source points (src) are assumed to be in the unit square (0,0) to (1,1).
// The resulting H is a 3x3 matrix such that for any point p in src (in homogeneous coordinates),
// H * p ~ q, where q is the corresponding destination point.
inline glm::mat3 computeHomography(const std::array<glm::vec2, 4>& src, const std::array<glm::vec2, 4>& dst) {
    // We want to solve for the 8 unknowns [h0, h1, h2, h3, h4, h5, h6, h7] in H,
    // where H is defined as:
    //      [ h0 h1 h2 ]
    // H =  [ h3 h4 h5 ]
    //      [ h6 h7  1 ]
    // For each correspondence (x, y) -> (x', y'), the two equations are:
    //   x * h0 + y * h1 +     h2 - x*x' * h6 - y*x' * h7 = x'
    //   x * h3 + y * h4 +     h5 - x*y' * h6 - y*y' * h7 = y'
    // We set up an 8x8 linear system A * x = b.
    float A[8][8] = { 0 };
    float b[8] = { 0 };

    for (int i = 0; i < 4; i++) {
        float x = src[i].x;
        float y = src[i].y;
        float x_prime = dst[i].x;
        float y_prime = dst[i].y;

        // Row for x equation
        A[2 * i][0] = x;
        A[2 * i][1] = y;
        A[2 * i][2] = 1.0f;
        A[2 * i][3] = 0.0f;
        A[2 * i][4] = 0.0f;
        A[2 * i][5] = 0.0f;
        A[2 * i][6] = -x * x_prime;
        A[2 * i][7] = -y * x_prime;
        b[2 * i] = x_prime;

        // Row for y equation
        A[2 * i + 1][0] = 0.0f;
        A[2 * i + 1][1] = 0.0f;
        A[2 * i + 1][2] = 0.0f;
        A[2 * i + 1][3] = x;
        A[2 * i + 1][4] = y;
        A[2 * i + 1][5] = 1.0f;
        A[2 * i + 1][6] = -x * y_prime;
        A[2 * i + 1][7] = -y * y_prime;
        b[2 * i + 1] = y_prime;
    }

    // Solve the 8x8 system using Gaussian elimination.
    float M[8][9]; // augmented matrix (8x8 and the b column)
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            M[i][j] = A[i][j];
        }
        M[i][8] = b[i];
    }

    for (int i = 0; i < 8; i++) {
        // Find the pivot row
        int pivot = i;
        for (int j = i + 1; j < 8; j++) {
            if (fabs(M[j][i]) > fabs(M[pivot][i]))
                pivot = j;
        }
        if (fabs(M[pivot][i]) < 1e-9)
            throw std::runtime_error("Singular matrix encountered in homography computation.");

        // Swap current row with pivot row
        if (pivot != i) {
            for (int j = 0; j < 9; j++) {
                std::swap(M[i][j], M[pivot][j]);
            }
        }
        // Normalize pivot row
        float factor = M[i][i];
        for (int j = i; j < 9; j++) {
            M[i][j] /= factor;
        }
        // Eliminate below
        for (int j = i + 1; j < 8; j++) {
            float factor2 = M[j][i];
            for (int k = i; k < 9; k++) {
                M[j][k] -= factor2 * M[i][k];
            }
        }
    }
    // Back substitution
    float h[8];
    for (int i = 7; i >= 0; i--) {
        h[i] = M[i][8];
        for (int j = i + 1; j < 8; j++) {
            h[i] -= M[i][j] * h[j];
        }
    }

    glm::mat3 H(1.0f);
    // Note: GLM uses column-major order.
    H[0][0] = h[0]; H[1][0] = h[1]; H[2][0] = h[2];
    H[0][1] = h[3]; H[1][1] = h[4]; H[2][1] = h[5];
    H[0][2] = h[6]; H[1][2] = h[7]; H[2][2] = 1.0f;
    return H;
}

#endif // HOMOGRAPHY_HPP
