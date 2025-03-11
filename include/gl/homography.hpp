#ifndef HOMOGRAPHY_HPP
#define HOMOGRAPHY_HPP

#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
#include <stdexcept>
#include <cmath>
#include <optional>

namespace gl {

    // Optimized LU decomposition solver for 8x8 systems
    class LinearSolver8x8 {
    private:
        float A[8][8];  // Matrix
        float b[8];     // Right-hand side
        int pivots[8];  // Pivot indices

    public:
        LinearSolver8x8() {
            // Initialize pivot array
            for (int i = 0; i < 8; ++i) {
                pivots[i] = i;
            }
        }

        // Set matrix and right-hand side values
        void setSystem(const float matrix[8][8], const float rhs[8]) {
            // Copy matrix and rhs
            for (int i = 0; i < 8; ++i) {
                for (int j = 0; j < 8; ++j) {
                    A[i][j] = matrix[i][j];
                }
                b[i] = rhs[i];
            }

            // Reset pivots
            for (int i = 0; i < 8; ++i) {
                pivots[i] = i;
            }
        }

        // Perform LU decomposition with partial pivoting
        bool decompose() {
            constexpr float EPSILON = 1e-10f;

            for (int i = 0; i < 8; ++i) {
                // Find pivot
                int pivot_row = i;
                float max_val = std::abs(A[pivots[i]][i]);

                for (int j = i + 1; j < 8; ++j) {
                    float val = std::abs(A[pivots[j]][i]);
                    if (val > max_val) {
                        max_val = val;
                        pivot_row = j;
                    }
                }

                // Check for singularity
                if (max_val < EPSILON) {
                    return false;
                }

                // Swap pivot rows if necessary
                if (pivot_row != i) {
                    std::swap(pivots[i], pivots[pivot_row]);
                }

                // Get pivot row index
                int pivot_idx = pivots[i];

                // For all rows below pivot
                for (int j = i + 1; j < 8; ++j) {
                    int row_idx = pivots[j];

                    // Calculate multiplier
                    float m = A[row_idx][i] / A[pivot_idx][i];
                    A[row_idx][i] = m;  // Store multiplier in the eliminated position

                    // Eliminate
                    for (int k = i + 1; k < 8; ++k) {
                        A[row_idx][k] -= m * A[pivot_idx][k];
                    }
                }
            }

            return true;
        }

        // Solve the system after decomposition
        void solve(float x[8]) {
            // Forward substitution (Ly = b)
            float y[8];
            for (int i = 0; i < 8; ++i) {
                int pivot_idx = pivots[i];
                y[i] = b[pivot_idx];

                for (int j = 0; j < i; ++j) {
                    y[i] -= A[pivot_idx][j] * y[j];
                }
            }

            // Backward substitution (Ux = y)
            for (int i = 7; i >= 0; --i) {
                int pivot_idx = pivots[i];
                x[i] = y[i];

                for (int j = i + 1; j < 8; ++j) {
                    x[i] -= A[pivot_idx][j] * x[j];
                }

                x[i] /= A[pivot_idx][i];
            }
        }
    };

    // Optimized homography computation
    class HomographyCalculator {
    private:
        LinearSolver8x8 solver;

        // Cache for last computed homography
        struct HomographyCache {
            std::array<glm::vec2, 4> src;
            std::array<glm::vec2, 4> dst;
            glm::mat3 matrix;
        };

        std::optional<HomographyCache> cache;

        bool arePointsSame(const std::array<glm::vec2, 4>& p1, const std::array<glm::vec2, 4>& p2, float epsilon = 1e-5f) {
            for (int i = 0; i < 4; ++i) {
                if (!glm::all(glm::epsilonEqual(p1[i], p2[i], epsilon))) {
                    return false;
                }
            }
            return true;
        }

    public:
        HomographyCalculator() = default;

        // Compute homography with optional caching
        glm::mat3 compute(const std::array<glm::vec2, 4>& src, const std::array<glm::vec2, 4>& dst, bool useCache = true) {
            // Check cache first if enabled
            if (useCache && cache.has_value()) {
                if (arePointsSame(src, cache->src) && arePointsSame(dst, cache->dst)) {
                    return cache->matrix;
                }
            }

            // Set up the 8x8 system for homography
            float A[8][8] = {};
            float b[8] = {};

            // Fill the matrix A and vector b with constraints from the point pairs
            for (int i = 0; i < 4; ++i) {
                float x = src[i].x;
                float y = src[i].y;
                float x_prime = dst[i].x;
                float y_prime = dst[i].y;

                // Row for x equation: x*h0 + y*h1 + h2 - x*x'*h6 - y*x'*h7 = x'
                A[i * 2][0] = x;
                A[i * 2][1] = y;
                A[i * 2][2] = 1.0f;
                A[i * 2][3] = 0.0f;
                A[i * 2][4] = 0.0f;
                A[i * 2][5] = 0.0f;
                A[i * 2][6] = -x * x_prime;
                A[i * 2][7] = -y * x_prime;
                b[i * 2] = x_prime;

                // Row for y equation: x*h3 + y*h4 + h5 - x*y'*h6 - y*y'*h7 = y'
                A[i * 2 + 1][0] = 0.0f;
                A[i * 2 + 1][1] = 0.0f;
                A[i * 2 + 1][2] = 0.0f;
                A[i * 2 + 1][3] = x;
                A[i * 2 + 1][4] = y;
                A[i * 2 + 1][5] = 1.0f;
                A[i * 2 + 1][6] = -x * y_prime;
                A[i * 2 + 1][7] = -y * y_prime;
                b[i * 2 + 1] = y_prime;
            }

            // Set up the solver
            solver.setSystem(A, b);

            // Perform LU decomposition
            if (!solver.decompose()) {
                throw std::runtime_error("Singular matrix encountered in homography computation");
            }

            // Solve the system
            float h[8];
            solver.solve(h);

            // Construct homography matrix (in column-major order for GLM)
            glm::mat3 H(1.0f);
            H[0][0] = h[0]; H[1][0] = h[1]; H[2][0] = h[2]; // First column
            H[0][1] = h[3]; H[1][1] = h[4]; H[2][1] = h[5]; // Second column
            H[0][2] = h[6]; H[1][2] = h[7]; H[2][2] = 1.0f; // Third column

            // Update cache if enabled
            if (useCache) {
                cache = HomographyCache{ src, dst, H };
            }

            return H;
        }

        // Clear the cache
        void clearCache() {
            cache.reset();
        }
    };

    // Global calculator instance
    static HomographyCalculator homographyCalculator;

    // External API for backward compatibility
    inline glm::mat3 computeHomography(const std::array<glm::vec2, 4>& src, const std::array<glm::vec2, 4>& dst) {
        return homographyCalculator.compute(src, dst);
    }

} // namespace gl

#endif // HOMOGRAPHY_HPP