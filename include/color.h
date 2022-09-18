#ifndef __COLOR_H__
#define __COLOR_H__

/**
 * A color to display on the screen.
 * The color is represented by its red, green, and blue components.
 * Each component must be between 0 (black) and 1 (white).
 */
typedef struct {
  float r;
  float g;
  float b;
} rgb_color_t;

/**
 * @brief Generates a random color
 *
 * @return rgb_color_t
 */
rgb_color_t rand_color();

/**
 * @brief Generates a random  purple color
 *
 * @return rgb_color_t
 */
rgb_color_t rand_purp_color();

#endif // #ifndef __COLOR_H__
