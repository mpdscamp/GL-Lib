#ifndef HOMOGRAPHY_HPP
#define HOMOGRAPHY_HPP

#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
#include <stdexcept>
#include <cmath>
#include <optional>
#include <limits>

#ifdef __AVX__
#include <immintrin.h> // For AVX intrinsics
#endif

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
#ifdef __AVX__
            // Forward substitution (Ly = b) with SIMD
            float y[8] = { 0 };

            for (int i = 0; i < 8; ++i) {
                int pivot_idx = pivots[i];
                y[i] = b[pivot_idx];

                if (i > 0) {
                    __m256 y_vec = _mm256_loadu_ps(y);
                    __m256 row_vec = _mm256_set_ps(
                        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
                    );

                    // Load only elements up to i-1 (others are 0)
                    for (int j = 0; j < i; ++j) {
                        float* row_ptr = reinterpret_cast<float*>(&row_vec);
                        row_ptr[j] = A[pivot_idx][j];
                    }

                    // Compute dot product
                    __m256 prod = _mm256_mul_ps(row_vec, y_vec);

                    // Horizontal sum
                    __m128 sum128 = _mm_add_ps(
                        _mm256_extractf128_ps(prod, 0),
                        _mm256_extractf128_ps(prod, 1)
                    );
                    sum128 = _mm_add_ps(sum128, _mm_movehl_ps(sum128, sum128));
                    sum128 = _mm_add_ss(sum128, _mm_shuffle_ps(sum128, sum128, 1));
                    float sum = _mm_cvtss_f32(sum128);

                    y[i] -= sum;
                }
            }

            // Backward substitution (Ux = y) with SIMD
            for (int i = 7; i >= 0; --i) {
                int pivot_idx = pivots[i];
                x[i] = y[i];

                if (i < 7) {
                    __m256 x_vec = _mm256_loadu_ps(x);
                    __m256 row_vec = _mm256_set_ps(
                        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
                    );

                    // Load only elements from i+1 to 7 (others are 0)
                    for (int j = i + 1; j < 8; ++j) {
                        float* row_ptr = reinterpret_cast<float*>(&row_vec);
                        row_ptr[j] = A[pivot_idx][j];
                    }

                    // Compute dot product
                    __m256 prod = _mm256_mul_ps(row_vec, x_vec);

                    // Horizontal sum
                    __m128 sum128 = _mm_add_ps(
                        _mm256_extractf128_ps(prod, 0),
                        _mm256_extractf128_ps(prod, 1)
                    );
                    sum128 = _mm_add_ps(sum128, _mm_movehl_ps(sum128, sum128));
                    sum128 = _mm_add_ss(sum128, _mm_shuffle_ps(sum128, sum128, 1));
                    float sum = _mm_cvtss_f32(sum128);

                    x[i] -= sum;
                }

                x[i] /= A[pivot_idx][i];
            }
#else
            // Standard non-SIMD implementation
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
#endif
        }
    };

    // Optimized homography computation
    class HomographyCalculator {
    private:
        LinearSolver8x8 solver;

        // Multi-level LRU cache for homographies
        struct HomographyCache {
            std::array<glm::vec2, 4> src;
            std::array<glm::vec2, 4> dst;
            glm::mat3 matrix;
            size_t lastUsed;
        };

        static constexpr size_t CACHE_SIZE = 4;
        std::array<std::optional<HomographyCache>, CACHE_SIZE> caches;
        size_t cacheCounter = 0;

        bool arePointsSame(const std::array<glm::vec2, 4>& p1,
            const std::array<glm::vec2, 4>& p2,
            float epsilon = 1e-5f) {
            for (int i = 0; i < 4; ++i) {
                if (!glm::all(glm::epsilonEqual(p1[i], p2[i], epsilon))) {
                    return false;
                }
            }
            return true;
        }

        // Find in cache or return nullopt
        std::optional<glm::mat3> findInCache(const std::array<glm::vec2, 4>& src,
            const std::array<glm::vec2, 4>& dst) {
            for (auto& cache : caches) {
                if (cache && arePointsSame(src, cache->src) &&
                    arePointsSame(dst, cache->dst)) {
                    cache->lastUsed = ++cacheCounter;
                    return cache->matrix;
                }
            }
            return std::nullopt;
        }

        // Add to cache, replacing least recently used entry
        void addToCache(const std::array<glm::vec2, 4>& src,
            const std::array<glm::vec2, 4>& dst,
            const glm::mat3& matrix) {
            // Find empty slot or least recently used
            size_t replaceIdx = 0;
            size_t minCounter = std::numeric_limits<size_t>::max();

            for (size_t i = 0; i < CACHE_SIZE; ++i) {
                if (!caches[i]) {
                    replaceIdx = i;
                    break;
                }

                if (caches[i]->lastUsed < minCounter) {
                    minCounter = caches[i]->lastUsed;
                    replaceIdx = i;
                }
            }

            caches[replaceIdx] = HomographyCache{
                src, dst, matrix, ++cacheCounter
            };
        }

        // Check if points form an axis-aligned rectangle
        bool isAxisAlignedRect(const std::array<glm::vec2, 4>& points, float epsilon = 1e-5f) {
            // Count unique x and y coordinates
            std::array<float, 4> xs = { points[0].x, points[1].x, points[2].x, points[3].x };
            std::array<float, 4> ys = { points[0].y, points[1].y, points[2].y, points[3].y };

            // Sort to find unique values
            std::sort(xs.begin(), xs.end());
            std::sort(ys.begin(), ys.end());

            // Remove duplicates (within epsilon)
            auto uniqueX = std::unique(xs.begin(), xs.end(),
                [epsilon](float a, float b) { return std::abs(a - b) < epsilon; });
            auto uniqueY = std::unique(ys.begin(), ys.end(),
                [epsilon](float a, float b) { return std::abs(a - b) < epsilon; });

            // If we have exactly 2 unique x and 2 unique y coordinates, it's an axis-aligned rectangle
            return (std::distance(xs.begin(), uniqueX) == 2 &&
                std::distance(ys.begin(), uniqueY) == 2);
        }

        // Compute simple transform for axis-aligned rectangles
        glm::mat3 computeSimpleTransform(const std::array<glm::vec2, 4>& src,
            const std::array<glm::vec2, 4>& dst) {
            // Find bounding box for src
            glm::vec2 srcMin(std::numeric_limits<float>::max());
            glm::vec2 srcMax(std::numeric_limits<float>::lowest());

            for (const auto& p : src) {
                srcMin = glm::min(srcMin, p);
                srcMax = glm::max(srcMax, p);
            }

            // Find bounding box for dst
            glm::vec2 dstMin(std::numeric_limits<float>::max());
            glm::vec2 dstMax(std::numeric_limits<float>::lowest());

            for (const auto& p : dst) {
                dstMin = glm::min(dstMin, p);
                dstMax = glm::max(dstMax, p);
            }

            // Calculate scale
            glm::vec2 srcSize = srcMax - srcMin;
            glm::vec2 dstSize = dstMax - dstMin;

            glm::vec2 scale(1.0f);
            if (std::abs(srcSize.x) > 1e-5f) scale.x = dstSize.x / srcSize.x;
            if (std::abs(srcSize.y) > 1e-5f) scale.y = dstSize.y / srcSize.y;

            // Calculate translation
            glm::vec2 translate = dstMin - srcMin * scale;

            // Create transformation matrix
            glm::mat3 H(1.0f);
            H[0][0] = scale.x;
            H[1][1] = scale.y;
            H[2][0] = translate.x;
            H[2][1] = translate.y;

            return H;
        }

    public:
        HomographyCalculator() = default;

        // Compute homography with optional caching
        glm::mat3 compute(const std::array<glm::vec2, 4>& src,
            const std::array<glm::vec2, 4>& dst,
            bool useCache = true) {
            // Check cache first if enabled
            if (useCache) {
                auto cachedMatrix = findInCache(src, dst);
                if (cachedMatrix) {
                    return *cachedMatrix;
                }
            }

            // Check for simple transformation case (axis-aligned rectangles)
            if (isAxisAlignedRect(src) && isAxisAlignedRect(dst)) {
                glm::mat3 H = computeSimpleTransform(src, dst);

                // Update cache if enabled
                if (useCache) {
                    addToCache(src, dst, H);
                }

                return H;
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

                // Precompute products to avoid redundant calculations
                float x_x_prime = x * x_prime;
                float y_x_prime = y * x_prime;
                float x_y_prime = x * y_prime;
                float y_y_prime = y * y_prime;

                // Row for x equation: x*h0 + y*h1 + h2 - x*x'*h6 - y*x'*h7 = x'
                A[i * 2][0] = x;
                A[i * 2][1] = y;
                A[i * 2][2] = 1.0f;
                A[i * 2][3] = 0.0f;
                A[i * 2][4] = 0.0f;
                A[i * 2][5] = 0.0f;
                A[i * 2][6] = -x_x_prime;
                A[i * 2][7] = -y_x_prime;
                b[i * 2] = x_prime;

                // Row for y equation: x*h3 + y*h4 + h5 - x*y'*h6 - y*y'*h7 = y'
                A[i * 2 + 1][0] = 0.0f;
                A[i * 2 + 1][1] = 0.0f;
                A[i * 2 + 1][2] = 0.0f;
                A[i * 2 + 1][3] = x;
                A[i * 2 + 1][4] = y;
                A[i * 2 + 1][5] = 1.0f;
                A[i * 2 + 1][6] = -x_y_prime;
                A[i * 2 + 1][7] = -y_y_prime;
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

            // Normalize the homography matrix for better numerical stability
            if (std::abs(H[2][2]) > 1e-10f) {
                float scale = 1.0f / H[2][2];
                for (int i = 0; i < 3; ++i) {
                    for (int j = 0; j < 3; ++j) {
                        H[i][j] *= scale;
                    }
                }
            }

            // Update cache if enabled
            if (useCache) {
                addToCache(src, dst, H);
            }

            return H;
        }

        // Clear the cache
        void clearCache() {
            for (auto& cache : caches) {
                cache.reset();
            }
            cacheCounter = 0;
        }
    };

    // Thread-local calculator for thread safety
    inline HomographyCalculator& getHomographyCalculator() {
        thread_local HomographyCalculator calculator;
        return calculator;
    }

    // External API for backward compatibility
    inline glm::mat3 computeHomography(const std::array<glm::vec2, 4>& src,
        const std::array<glm::vec2, 4>& dst) {
        return getHomographyCalculator().compute(src, dst);
    }

} // namespace gl

#endif // HOMOGRAPHY_HPP