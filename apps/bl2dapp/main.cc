#include <blend2d.h>
#include <cairomm/cairomm.h>
#include <chrono>
#include <iostream>
void testcairo(int loops){
    Cairo::RefPtr<Cairo::ImageSurface> surface =
        Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32,1280, 480);

    Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(surface);

    Cairo::RefPtr<Cairo::LinearGradient> gradient =
    Cairo::LinearGradient::create(0, 0, 1280, 480);
    gradient->add_color_stop_rgb(0,1,1,1);
    gradient->add_color_stop_rgb(0.5,0,0,1);
    gradient->add_color_stop_rgb(1, 1,0,0);

    cr->set_source(gradient);
    auto start = std::chrono::high_resolution_clock::now();
    for(int i=0;i<loops;i++){
        cr->rectangle(0, 0, 1280, 480);
        cr->fill();
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << __FUNCTION__<<" "<< duration.count() << " seconds" << std::endl;
    surface->write_to_png("linear_gradient.png");

    std::cout << "PNG image saved as linear_gradient.png" << std::endl;
}
void testblend2d(int loops) {
  BLImage img(1280, 480, BL_FORMAT_PRGB32);
  BLContext ctx(img);

  ctx.clearAll();

  // Coordinates can be specified now or changed
  // later via BLGradient accessors.
  BLGradient linear(
  BLLinearGradientValues(0, 0, 1280, 480));

  // Color stops can be added in any order.
  linear.addStop(0.0, BLRgba32(0xFFFFFFFF));
  linear.addStop(0.5, BLRgba32(0xFF0000FF));
  linear.addStop(1.0, BLRgba32(0xFFFF0000));

  // `setFillStyle()` can be used for both colors
  // and styles. Alternatively, a color or style
  // can be passed explicitly to a render function.
  ctx.setFillStyle(linear);

  // Rounded rect will be filled with the linear
  // gradient.
  auto start = std::chrono::high_resolution_clock::now();
  for(int i=0;i<loops;i++)
     ctx.fillRect(0,0, 1280, 480);
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> duration = end - start;

  std::cout << __FUNCTION__<<" "<< duration.count() << " seconds" << std::endl;
  ctx.end();
  img.writeToFile("bl_sample_1.png");
}
int main(int argc, char* argv[]){
   int loops=argc>1?atoi(argv[1]):10000;
   std::cout<<"Cairo/Blend2d bench test"<<std::endl;
   testblend2d(loops);
   testcairo  (loops);
}
