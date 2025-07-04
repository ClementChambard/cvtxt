%format = A4 portrait;

banner_box_height = 25%;

%style banner_style = {
  background_color = #abcdef;
  text_color = #fedcba;
};

%style right_box_style = {};
%style left_box_style = {};

%style right_style = {
  background_color = #abcdef;
  text_color = #fedcba;
};

%layout = layers {
  hsplit (loc = 70%) {
    column {
      box (h = $banner_box_height),
      box description (style = $left_box_style),
      box formation (style = $left_box_style),
      box exp_pro (style = $left_box_style),
      box exp_perso (style = $left_box_style)
    },
    column (style = $right_style) {
      box (h = $banner_box_height),
      box coords (style = $right_box_style),
      box capa (style = $right_box_style),
      box lang (style = $right_box_style),
      box hobby (style = $right_box_style)
    }
  },
  box (h = $banner_box_height, py = 10%) {
    box banner (style = $banner_style)
  }
};
