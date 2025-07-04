banner_box_height = 20%;

bg_color_light = #98cecd;
bg_color_medium = #43868f;
bg_color_dark = #385b5f;

%style banner_style = {
  margin_t = 3vw;
  background_color = $bg_color_dark;
};

%style right_box_style = {
  background_color = $bg_color_dark;
  margin_b = 80%;
  corner_radius = 20;
};

%style left_box_style = {
  background_color = $bg_color_light;
  corner_radius = 20;
};

%style right_style = {
  gap = 2vw;
  margin_t = $banner_box_height;
  padding_y = 4vw;
  padding_x = 3vw;
};

%style left_style = {
  margin_t = $banner_box_height;
  padding_x = 3vw; 
  padding_y = 3vw;
  gap = 3vw;
};

%layout = layers {
  hsplit (loc = 70%) {
    column (style = $left_style){
      box description (style = $left_box_style, h = 7%),
      box formation (style = $left_box_style, h = 13%),
      box exp_pro (style = $left_box_style),
      box exp_perso (style = $left_box_style)
    },
    box (background_color = $bg_color_medium) {
      column (style = $right_style) {
        box coords (style = $right_box_style),
        box capa (style = $right_box_style),
        box lang (style = $right_box_style),
        box hobby (style = $right_box_style)
      }
    }
  },
  box (h = $banner_box_height) {
    box banner (style = $banner_style)
  }
};
