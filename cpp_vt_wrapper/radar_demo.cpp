#include "radar_demo.h"

#include <chrono>
#include <s9w/s9w_rng.h>
#include <s9w/s9w_geom_types.h>
#include <s9w/s9w_geom_alg.h>

#include "../wrapper.h"
using namespace cvtsw;
#include "tools.h"

static s9w::rng_state rng;

namespace
{
   auto get_faded(const color& col) -> color {
      constexpr int fade_amount = 20;
      return color{
         static_cast<uint8_t>(std::clamp(col.red - fade_amount, 0, 255)),
         static_cast<uint8_t>(std::clamp(col.green - fade_amount, 0, 255)),
         static_cast<uint8_t>(std::clamp(col.blue - fade_amount, 0, 255))
      };
   };

   template<typename T>
   auto nonstupid_atan2(const T y, const T x){
      T result = std::atan2(y, x);
      if (result < 0.0)
         result += 2.0 * std::numbers::pi_v<double>;
      return result;
   }
}

auto radar_demo() -> void
{
   constexpr int width = 60;
   constexpr int height = 60;
   constexpr int pixel_count = width * height;
   pixel_screen px{ width, height, 0, 0, color{0, 0, 0} };
   constexpr s9w::dvec2 center{ width / 2.0, height / 2.0 };
   constexpr s9w::dvec2 half_pixel_offset{ 0.5, 0.5 };
   constexpr double arm_length = height / 2.0 - 5.0;

   timer timer;
   while(true){

      // Fading of all pixels to black
      const int dim_runs = get_int(0.1 * pixel_count);
      for(int i=0; i< dim_runs; ++i){
         color& choice = rng.choice(px.m_pixels);
         choice = get_faded(choice);
      }

      // Radar arm
      constexpr double speed = 3.0;
      const double radar_phi = std::fmod(speed * timer.get_seconds_since_start(), 2.0 * std::numbers::pi_v<double>);
      for(int y=0; y<height; ++y){
         for (int x = 0; x < width; ++x){
            const s9w::dvec2 pos = s9w::dvec2{ x, y } - center + half_pixel_offset;
            const double radius = s9w::get_length(pos);
            const double pixel_phi = nonstupid_atan2(pos[1], pos[0]);
            if(std::abs(pixel_phi-radar_phi)<0.05 && radius < arm_length)
               px.get_color(x, y) = color{ 255, 0, 0 };
         }
      }

      // Printing
      const std::wstring result_str = px.get_screen(color{ 0, 255, 0 }).get_string();
      
      timer.mark_frame();

      fast_print(result_str);
      const auto fps = timer.get_fps();
      if(fps.has_value())
         set_window_title("FPS: " + std::to_string(*fps));
   }
}
