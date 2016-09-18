#ifdef CETECH_TEST

/*******************************************************************************
**** Includes
*******************************************************************************/

#include "include/catch/catch.hpp"

extern "C" {
#include "../types.h"
#include "../vec3f.h"
};

/*******************************************************************************
**** Test helpers
*******************************************************************************/

/*******************************************************************************
**** Basic vector operation
*******************************************************************************/
SCENARIO( "Basic vec3f operation", "[math]" ) {
    GIVEN( "v1 = [1.0f, 2.0f, 3.0f] and v2 = [4.0f, 5.0f, 6.0f]" ) {

        vec3f_t v1 = {1.0f, 2.0f, 3.0f};
        vec3f_t v2 = {4.0f, 5.0f, 6.0f};

        WHEN("v1 + v2") {
            vec3f_t result = {0};

            vec3f_add(&result, &v1, &v2);

            THEN("result == [5.0f, 7.0f, 9.0f]") {
                REQUIRE(result.x == 5.0f);
                REQUIRE(result.y == 7.0f);
                REQUIRE(result.z == 9.0f);
            }
        }

        WHEN("v1 - v2") {
            vec3f_t result = {0};

            vec3f_sub(&result, &v1, &v2);

            THEN("result == [-3.0f, -3.0f, -3.0f]") {
                REQUIRE(result.x == -3.0f);
                REQUIRE(result.y == -3.0f);
                REQUIRE(result.z == -3.0f);
            }
        }

        WHEN("v1 * 2.0f") {
            vec3f_t result = {0};

            vec3f_mul(&result, &v1, 2.0f);

            THEN("result == [2.0f, 4.0f, 6.0f]") {
                REQUIRE(result.x == 2.0f);
                REQUIRE(result.y == 4.0f);
                REQUIRE(result.z == 6.0f);
            }
        }

        WHEN("v1 / 2.0f") {
            vec3f_t result = {0};

            vec3f_div(&result, &v1, 2.0f);

            THEN("result == [0.5f, 1.0f]") {
                REQUIRE(result.x == 0.5f);
                REQUIRE(result.y == 1.0f);
                REQUIRE(result.z == 1.5f);
            }
        }

        WHEN("v1 dot v2") {
            float dot = vec3f_dot(&v1, &v2);

            THEN("dot == 32.0f") {
                REQUIRE( dot == 32.0f);
            }
        }

        WHEN("len_squared v1") {
            float len_sq = vec3f_length_squared(&v1);

            THEN("len_sq == 14.0f") {
                REQUIRE( len_sq == 14.0f);
            }
        }

        WHEN("len v1") {
            float len_sq = vec3f_length(&v1);

            THEN("len == 3.74166f") {
                REQUIRE( len_sq == Approx(3.74166f));
            }
        }

        WHEN("normalize v1") {
            vec3f_t result = {0};
            vec3f_normalized(&result, &v1);

            THEN("result == [~0.26726f, ~0.53452f, ~0.80178f]") {
                REQUIRE(result.x == Approx(0.26726f));
                REQUIRE(result.y == Approx(0.53452f));
                REQUIRE(result.z == Approx(0.80178f));
            }
        }

        WHEN("v1 eq v2") {
            THEN("v1 != v2") {
                REQUIRE_FALSE(vec3f_eq(&v1, &v2, f32_Epsilon));
            }
        }

        WHEN("v1 eq v1") {
            THEN("v1 == v1") {
                REQUIRE(vec3f_eq(&v1, &v1, f32_Epsilon));
            }
        }

    }
}

#endif