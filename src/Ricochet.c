#include "pebble_os.h"
#include "pebble_app.h"

#define MY_UUID {0x7e, 0x0a, 0x8d, 0xaa, 0x84, 0x8f, 0x48, 0xe0, 0x9c, 0x44, 0x2d, 0x27, 0xb2, 0x75, 0xc7, 0xdb}
PBL_APP_INFO(MY_UUID,
	"Ricochet", "Pebble Technology & KD5RXT",
	1, 0, /* App major/minor version */
	RESOURCE_ID_IMAGE_MENU_ICON,
	APP_INFO_STANDARD_APP);

Window window;


#define TOTAL_TIME_DIGITS 6
#define TOTAL_DATE_DIGITS 8

bool night_enabled = false;
bool clock_24h_style;
bool date_month_first = true;
bool time_on_top = false;

int splash_timer = 3;
int freeze_timer;

int time_x_max;

int time_x_delta;
int time_y_delta;

int time_x_offset;
int time_y_offset;

int date_x_max;

int date_x_delta;
int date_y_delta;

int date_x_offset;
int date_y_offset;

BmpContainer time_digits_images[TOTAL_TIME_DIGITS];
BmpContainer day_image;
BmpContainer date_images[TOTAL_DATE_DIGITS];
BmpContainer splash_image;

const int BIG_DIGIT_IMAGE_RESOURCE_IDS[] =
{
   RESOURCE_ID_IMAGE_NUM_0,
   RESOURCE_ID_IMAGE_NUM_1,
   RESOURCE_ID_IMAGE_NUM_2,
   RESOURCE_ID_IMAGE_NUM_3,
   RESOURCE_ID_IMAGE_NUM_4,
   RESOURCE_ID_IMAGE_NUM_5,
   RESOURCE_ID_IMAGE_NUM_6,
   RESOURCE_ID_IMAGE_NUM_7,
   RESOURCE_ID_IMAGE_NUM_8,
   RESOURCE_ID_IMAGE_NUM_9,
};

const int BIG_DIGIT_INV_IMAGE_RESOURCE_IDS[] =
{
   RESOURCE_ID_IMAGE_NUM_INV_0,
   RESOURCE_ID_IMAGE_NUM_INV_1,
   RESOURCE_ID_IMAGE_NUM_INV_2,
   RESOURCE_ID_IMAGE_NUM_INV_3,
   RESOURCE_ID_IMAGE_NUM_INV_4,
   RESOURCE_ID_IMAGE_NUM_INV_5,
   RESOURCE_ID_IMAGE_NUM_INV_6,
   RESOURCE_ID_IMAGE_NUM_INV_7,
   RESOURCE_ID_IMAGE_NUM_INV_8,
   RESOURCE_ID_IMAGE_NUM_INV_9,
};


const int DATENUM_IMAGE_RESOURCE_IDS[] =
{
   RESOURCE_ID_IMAGE_DATENUM_0,
   RESOURCE_ID_IMAGE_DATENUM_1,
   RESOURCE_ID_IMAGE_DATENUM_2,
   RESOURCE_ID_IMAGE_DATENUM_3,
   RESOURCE_ID_IMAGE_DATENUM_4,
   RESOURCE_ID_IMAGE_DATENUM_5,
   RESOURCE_ID_IMAGE_DATENUM_6,
   RESOURCE_ID_IMAGE_DATENUM_7,
   RESOURCE_ID_IMAGE_DATENUM_8,
   RESOURCE_ID_IMAGE_DATENUM_9,
};

const int DATENUM_INV_IMAGE_RESOURCE_IDS[] =
{
   RESOURCE_ID_IMAGE_DATENUM_INV_0,
   RESOURCE_ID_IMAGE_DATENUM_INV_1,
   RESOURCE_ID_IMAGE_DATENUM_INV_2,
   RESOURCE_ID_IMAGE_DATENUM_INV_3,
   RESOURCE_ID_IMAGE_DATENUM_INV_4,
   RESOURCE_ID_IMAGE_DATENUM_INV_5,
   RESOURCE_ID_IMAGE_DATENUM_INV_6,
   RESOURCE_ID_IMAGE_DATENUM_INV_7,
   RESOURCE_ID_IMAGE_DATENUM_INV_8,
   RESOURCE_ID_IMAGE_DATENUM_INV_9,
};

const int DAY_IMAGE_RESOURCE_IDS[] =
{
   RESOURCE_ID_IMAGE_DAY_SUN,
   RESOURCE_ID_IMAGE_DAY_MON,
   RESOURCE_ID_IMAGE_DAY_TUE,
   RESOURCE_ID_IMAGE_DAY_WED,
   RESOURCE_ID_IMAGE_DAY_THU,
   RESOURCE_ID_IMAGE_DAY_FRI,
   RESOURCE_ID_IMAGE_DAY_SAT,
};


const int DAY_INV_IMAGE_RESOURCE_IDS[] =
{
   RESOURCE_ID_IMAGE_DAY_INV_SUN,
   RESOURCE_ID_IMAGE_DAY_INV_MON,
   RESOURCE_ID_IMAGE_DAY_INV_TUE,
   RESOURCE_ID_IMAGE_DAY_INV_WED,
   RESOURCE_ID_IMAGE_DAY_INV_THU,
   RESOURCE_ID_IMAGE_DAY_INV_FRI,
   RESOURCE_ID_IMAGE_DAY_INV_SAT,
};


void click_config_provider(ClickConfig **config, Window *window);
void down_single_click_handler(ClickRecognizerRef recognizer, Window *window);
void handle_deinit(AppContextRef ctx);
void handle_init(AppContextRef ctx);
void handle_second_tick(AppContextRef ctx, PebbleTickEvent *t);
void select_long_click_handler(ClickRecognizerRef recognizer, Window *window);
void select_long_release_handler(ClickRecognizerRef recognizer, Window *window);
void select_single_click_handler(ClickRecognizerRef recognizer, Window *window);
void set_container_image(BmpContainer *bmp_container, const int resource_id, GPoint origin);
void up_single_click_handler(ClickRecognizerRef recognizer, Window *window);
void update_date(PblTm *current_time);
void update_display(PblTm *current_time, bool move);
void update_moves(PblTm *current_time);
void update_time(PblTm *current_time);


void click_config_provider(ClickConfig **config, Window *window)
{
   (void)window;

   config[BUTTON_ID_SELECT]->click.handler = (ClickHandler) select_single_click_handler;

   config[BUTTON_ID_SELECT]->long_click.handler = (ClickHandler) select_long_click_handler;
   config[BUTTON_ID_SELECT]->long_click.release_handler = (ClickHandler) select_long_release_handler;

   config[BUTTON_ID_UP]->click.handler = (ClickHandler) up_single_click_handler;
   config[BUTTON_ID_UP]->click.repeat_interval_ms = 100;

   config[BUTTON_ID_DOWN]->click.handler = (ClickHandler) down_single_click_handler;
   config[BUTTON_ID_DOWN]->click.repeat_interval_ms = 100;
}  // click_config_provider()


void down_single_click_handler(ClickRecognizerRef recognizer, Window *window)
{
   PblTm current_time;

   (void)recognizer;
   (void)window;

   clock_24h_style = !clock_24h_style;

   if (clock_24h_style)
   {
      time_x_max = 93;
   }
   else
   {
      time_x_max = 103;
   }

   get_time(&current_time);
   update_display(&current_time, false);
}  // down_single_click_handler()


void handle_deinit(AppContextRef ctx)
{
   (void)ctx;

   for (int i = 0; i < TOTAL_TIME_DIGITS; i++)
   {
      bmp_deinit_container(&time_digits_images[i]);
   }
   for (int i = 0; i < TOTAL_DATE_DIGITS; i++)
   {
      bmp_deinit_container(&date_images[i]);
   }
   bmp_deinit_container(&day_image);
   bmp_deinit_container(&splash_image);
}  // handle_deinit()


void handle_init(AppContextRef ctx)
{
   (void)ctx;

   // version 1.1 of SDK requires vars to be intialized manually
   // START manual var intialization

   night_enabled = false;
   date_month_first = true;
   time_on_top = false;

   splash_timer = 3;
   freeze_timer = 5;

   if (clock_is_24h_style())
   {
      clock_24h_style = true;
      time_x_max = 93;
   }
   else
   {
      clock_24h_style = false;
      time_x_max = 103;
   }

   time_x_offset = 0;
   time_y_offset = 0;

   time_x_delta = 2;
   time_y_delta = 3;

   date_x_max = 104;
   date_x_offset = 0;
   date_y_offset = 0;

   date_x_delta = -3;
   date_y_delta = -2;
   // END manual var intialization

   window_init(&window, "Ricochet");
   window_set_fullscreen(&window, true);
   window_stack_push(&window, true /* Animated */);

   resource_init_current_app(&APP_RESOURCES);

   // Attach custom button functionality
   window_set_click_config_provider(&window, (ClickConfigProvider) click_config_provider);

   // version 1.1 of SDK requires vars to be intialized manually
   // START manual var intialization
   for (int i = 0; i < TOTAL_TIME_DIGITS; i++)
   {
      bmp_init_container(RESOURCE_ID_IMAGE_NUM_BLANK, &time_digits_images[i]);
   }
   for (int i = 0; i < TOTAL_DATE_DIGITS; i++)
   {
      bmp_init_container(RESOURCE_ID_IMAGE_DATENUM_SLASH, &date_images[i]);
   }
   bmp_init_container(RESOURCE_ID_IMAGE_SPLASH, &splash_image);
   bmp_init_container(RESOURCE_ID_IMAGE_DAY_SUN, &day_image);

   // END manual var intialization

}  // handle_init()


void handle_second_tick(AppContextRef ctx, PebbleTickEvent *t)
{
   (void)ctx;

   if (splash_timer > 0)
   {
      splash_timer--;

      if (splash_timer != 0)
      {
         set_container_image(&splash_image, RESOURCE_ID_IMAGE_SPLASH, GPoint(0, 0));
         return;
      }
      else
      {
         layer_remove_from_parent(&splash_image.layer.layer);
         bmp_deinit_container(&splash_image);
      }
   }

   if (freeze_timer > 0)
   {
      freeze_timer--;
   }
   update_display(t->tick_time, true);
}  // handle_second_tick()


void select_long_click_handler(ClickRecognizerRef recognizer, Window *window)
{
   PblTm current_time;

   (void)recognizer;
   (void)window;

   if (splash_timer == 0)
   {
      night_enabled = !night_enabled;
   }

   get_time(&current_time);
   update_display(&current_time, false);
}  // select_long_click_handler()


void select_long_release_handler(ClickRecognizerRef recognizer, Window *window)
{
   (void)recognizer;
   (void)window;
}  // select_long_release_handler()


void select_single_click_handler(ClickRecognizerRef recognizer, Window *window)
{
   PblTm current_time;

   (void)recognizer;
   (void)window;

   time_on_top = !time_on_top;

   freeze_timer = 5;

   get_time(&current_time);
   update_display(&current_time, false);
}  // select_single_click_handler()


void set_container_image(BmpContainer *bmp_container, const int resource_id, GPoint origin)
{
   layer_remove_from_parent(&bmp_container->layer.layer);
   bmp_deinit_container(bmp_container);

   bmp_init_container(resource_id, bmp_container);

   GRect frame = layer_get_frame(&bmp_container->layer.layer);
   frame.origin.x = origin.x;
   frame.origin.y = origin.y;
   layer_set_frame(&bmp_container->layer.layer, frame);

   layer_add_child(&window.layer, &bmp_container->layer.layer);
}  // set_container_image()


void up_single_click_handler(ClickRecognizerRef recognizer, Window *window)
{
   PblTm current_time;

   (void)recognizer;
   (void)window;

   date_month_first = !date_month_first;

   get_time(&current_time);
   update_display(&current_time, false);
}  // up_single_click_handler()


void update_date(PblTm *current_time)
{
   if (freeze_timer > 0)
   {
      if (time_on_top)
      {
         date_x_offset = 20;
         date_y_offset = 75;
      }
      else
      {
         date_x_offset = 20;
         date_y_offset = 10;
      }
   }

   // display date
   if (night_enabled == false)
   {
      set_container_image(&day_image, DAY_IMAGE_RESOURCE_IDS[current_time->tm_wday], GPoint(date_x_offset + 30, date_y_offset));

      if (date_month_first)
      {
         set_container_image(&date_images[0], DATENUM_IMAGE_RESOURCE_IDS[(current_time->tm_mon + 1) / 10], GPoint(date_x_offset, date_y_offset + 23));
         set_container_image(&date_images[1], DATENUM_IMAGE_RESOURCE_IDS[(current_time->tm_mon + 1) % 10], GPoint(date_x_offset + 13, date_y_offset + 23));
         set_container_image(&date_images[3], DATENUM_IMAGE_RESOURCE_IDS[current_time->tm_mday / 10], GPoint(date_x_offset + 39, date_y_offset + 23));
         set_container_image(&date_images[4], DATENUM_IMAGE_RESOURCE_IDS[current_time->tm_mday % 10], GPoint(date_x_offset + 52, date_y_offset + 23));
      }
      else
      {
         set_container_image(&date_images[3], DATENUM_IMAGE_RESOURCE_IDS[current_time->tm_mday / 10], GPoint(date_x_offset, date_y_offset + 23));
         set_container_image(&date_images[4], DATENUM_IMAGE_RESOURCE_IDS[current_time->tm_mday % 10], GPoint(date_x_offset + 13, date_y_offset + 23));
         set_container_image(&date_images[0], DATENUM_IMAGE_RESOURCE_IDS[(current_time->tm_mon + 1) / 10], GPoint(date_x_offset + 39, date_y_offset + 23));
         set_container_image(&date_images[1], DATENUM_IMAGE_RESOURCE_IDS[(current_time->tm_mon + 1) % 10], GPoint(date_x_offset + 52, date_y_offset + 23));
      }

      set_container_image(&date_images[6], DATENUM_IMAGE_RESOURCE_IDS[(current_time->tm_year / 10) % 10], GPoint(date_x_offset + 78, date_y_offset + 23));
      set_container_image(&date_images[7], DATENUM_IMAGE_RESOURCE_IDS[current_time->tm_year % 10], GPoint(date_x_offset + 91, date_y_offset + 23));
      set_container_image(&date_images[2], RESOURCE_ID_IMAGE_DATENUM_SLASH, GPoint(date_x_offset + 26, date_y_offset + 23));
      set_container_image(&date_images[5], RESOURCE_ID_IMAGE_DATENUM_SLASH, GPoint(date_x_offset + 65, date_y_offset + 23));
   }
   else
   {
      set_container_image(&day_image, DAY_INV_IMAGE_RESOURCE_IDS[current_time->tm_wday], GPoint(date_x_offset + 30, date_y_offset));

      if (date_month_first)
      {
         set_container_image(&date_images[0], DATENUM_INV_IMAGE_RESOURCE_IDS[(current_time->tm_mon + 1) / 10], GPoint(date_x_offset, date_y_offset + 23));
         set_container_image(&date_images[1], DATENUM_INV_IMAGE_RESOURCE_IDS[(current_time->tm_mon + 1) % 10], GPoint(date_x_offset + 13, date_y_offset + 23));
         set_container_image(&date_images[3], DATENUM_INV_IMAGE_RESOURCE_IDS[current_time->tm_mday / 10], GPoint(date_x_offset + 39, date_y_offset + 23));
         set_container_image(&date_images[4], DATENUM_INV_IMAGE_RESOURCE_IDS[current_time->tm_mday % 10], GPoint(date_x_offset + 52, date_y_offset + 23));
      }
      else
      {
         set_container_image(&date_images[3], DATENUM_INV_IMAGE_RESOURCE_IDS[current_time->tm_mday / 10], GPoint(date_x_offset, date_y_offset + 23));
         set_container_image(&date_images[4], DATENUM_INV_IMAGE_RESOURCE_IDS[current_time->tm_mday % 10], GPoint(date_x_offset + 13, date_y_offset + 23));
         set_container_image(&date_images[0], DATENUM_INV_IMAGE_RESOURCE_IDS[(current_time->tm_mon + 1) / 10], GPoint(date_x_offset + 39, date_y_offset + 23));
         set_container_image(&date_images[1], DATENUM_INV_IMAGE_RESOURCE_IDS[(current_time->tm_mon + 1) % 10], GPoint(date_x_offset + 52, date_y_offset + 23));
      }

      set_container_image(&date_images[6], DATENUM_INV_IMAGE_RESOURCE_IDS[(current_time->tm_year / 10) % 10], GPoint(date_x_offset + 78, date_y_offset + 23));
      set_container_image(&date_images[7], DATENUM_INV_IMAGE_RESOURCE_IDS[current_time->tm_year % 10], GPoint(date_x_offset + 91, date_y_offset + 23));
      set_container_image(&date_images[2], RESOURCE_ID_IMAGE_DATENUM_INV_SLASH, GPoint(date_x_offset + 26, date_y_offset + 23));
      set_container_image(&date_images[5], RESOURCE_ID_IMAGE_DATENUM_INV_SLASH, GPoint(date_x_offset + 65, date_y_offset + 23));
   }
}  // update_date()


void update_display(PblTm *current_time, bool move)
{
   if (move)
   {
      update_moves(current_time);
   }

   if (time_on_top)
   {
      update_date(current_time);
      update_time(current_time);
   }
   else
   {
      update_time(current_time);
      update_date(current_time);
   }
}  // update_display()


void update_moves(PblTm *current_time)
{
   float x, y;

   date_x_offset += date_x_delta;
   date_y_offset += date_y_delta;

   time_x_offset += time_x_delta;
   time_y_offset += time_y_delta;

   // total date field is 104w x 39h
   if ((date_x_offset + date_x_delta) < 0)
   {
      // generate a pseudo random number from 2, 4, & 6
      x =  (sin_lookup (current_time->tm_hour + current_time->tm_min + current_time->tm_sec + 3)) * 1000;
      date_x_delta = (int) x;
      date_x_delta = ((date_x_delta % 3) + 1) * 2;
   }
   else
   {
      if ((date_x_offset + date_x_delta + date_x_max) >= 144)
      {
         // generate a pseudo random number from -2, -4, & -6
         x =  (cos_lookup (current_time->tm_hour + current_time->tm_min + current_time->tm_sec + 7)) * 1000;
         date_x_delta = (int) x;
         date_x_delta = ((date_x_delta % 3) + 1) * 2;
         date_x_delta = -date_x_delta;
      }
   }

   // total time field is 103w x 52h for 12-hour clock & 93w x 52h for 24-hour clock
   if ((time_x_offset + time_x_delta) < 0)
   {
      // generate a pseudo random number from 2, 4, & 6
      x =  (sin_lookup (current_time->tm_hour + current_time->tm_min + current_time->tm_sec + 19)) * 1000;
      time_x_delta = (int) x;
      time_x_delta = ((time_x_delta % 3) + 1) * 2;
   }
   else
   {
      if ((time_x_offset + time_x_delta + time_x_max) >= 144)
      {
         // generate a pseudo random number from -2, -4, & -6
         x =  (cos_lookup (current_time->tm_hour + current_time->tm_min + current_time->tm_sec + 37)) * 1000;
         time_x_delta = (int) x;
         time_x_delta = ((time_x_delta % 3) + 1) * 2;
         time_x_delta = -time_x_delta;
      }
   }

   if (time_on_top == true)
   {
      if ((time_y_offset + time_y_delta) < 0)
      {
         // generate a pseudo random number from 3, 6, & 9
         y =  (sin_lookup (current_time->tm_hour + current_time->tm_min + current_time->tm_sec + 53)) * 1000;
         time_y_delta = (int) y;
         time_y_delta = ((time_y_delta % 3) + 1) * 3;
      }

      if ((date_y_offset + date_y_delta + 39) >= 168)
      {
         // generate a pseudo random number from -4, -8, & -12
         y =  (sin_lookup (current_time->tm_hour + current_time->tm_min + current_time->tm_sec + 11)) * 1000;
         date_y_delta = (int) y;
         date_y_delta = ((date_y_delta % 3) + 1) * 4;
         date_y_delta = -date_y_delta;
      }

      if (((date_y_offset + date_y_delta) - (time_y_offset + time_y_delta)) <= 52)
      {
         // generate a pseudo random number from -3, -6, & -9
         y =  (sin_lookup (current_time->tm_hour + current_time->tm_min + current_time->tm_sec + 53)) * 1000;
         time_y_delta = (int) y;
         time_y_delta = ((time_y_delta % 3) + 1) * 3;
         time_y_delta = -time_y_delta;

         // generate a pseudo random number from 4, 8, & 12
         y =  (cos_lookup (current_time->tm_hour + current_time->tm_min + current_time->tm_sec + 9)) * 1000;
         date_y_delta = (int) y;
         date_y_delta = ((date_y_delta % 3) + 1) * 4;
      }
   }
   else
   {
      if ((date_y_offset + date_y_delta) < 0)
      {
         // generate a pseudo random number from 4, 8, & 12
         y =  (sin_lookup (current_time->tm_hour + current_time->tm_min + current_time->tm_sec + 11)) * 1000;
         date_y_delta = (int) y;
         date_y_delta = ((date_y_delta % 3) + 1) * 4;
      }

      if ((time_y_offset + time_y_delta + 52) >= 168)
      {
         // generate a pseudo random number from -3, -6, & -9
         y =  (cos_lookup (current_time->tm_hour + current_time->tm_min + current_time->tm_sec + 9)) * 1000;
         time_y_delta = (int) y;
         time_y_delta = ((time_y_delta % 3) + 1) * 3;
         time_y_delta = -time_y_delta;
      }

      if (((time_y_offset + time_y_delta) - (date_y_offset + date_y_delta)) <= 39)
      {
         // generate a pseudo random number from 3, 6, & 9
         y =  (sin_lookup (current_time->tm_hour + current_time->tm_min + current_time->tm_sec + 53)) * 1000;
         time_y_delta = (int) y;
         time_y_delta = ((time_y_delta % 3) + 1) * 3;

         // generate a pseudo random number from -4, -8, & -12
         y =  (cos_lookup (current_time->tm_hour + current_time->tm_min + current_time->tm_sec + 9)) * 1000;
         date_y_delta = (int) y;
         date_y_delta = ((date_y_delta % 3) + 1) * 4;
         date_y_delta = -date_y_delta;
      }
   }
}  // update_moves()


void update_time(PblTm *current_time)
{
   float x, y;

   if (freeze_timer > 0)
   {
      if (time_on_top)
      {
         time_x_offset = 20;
         time_y_offset = 10;
      }
      else
      {
         time_x_offset = 20;
         time_y_offset = 75;
      }
   }

   // display time hour
   if (clock_24h_style)
   {
      if (night_enabled == false)
      {
         set_container_image(&time_digits_images[0], BIG_DIGIT_IMAGE_RESOURCE_IDS[current_time->tm_hour / 10], GPoint(time_x_offset, time_y_offset));
         set_container_image(&time_digits_images[1], BIG_DIGIT_IMAGE_RESOURCE_IDS[current_time->tm_hour % 10], GPoint(21 + time_x_offset, time_y_offset));
      }
      else
      {
         set_container_image(&time_digits_images[0], BIG_DIGIT_INV_IMAGE_RESOURCE_IDS[current_time->tm_hour / 10], GPoint(time_x_offset, time_y_offset));
         set_container_image(&time_digits_images[1], BIG_DIGIT_INV_IMAGE_RESOURCE_IDS[current_time->tm_hour % 10], GPoint(21 + time_x_offset, time_y_offset));
      }

      layer_remove_from_parent(&time_digits_images[5].layer.layer);
      bmp_deinit_container(&time_digits_images[5]);
   }
   else
   {
      // display AM/PM
      if (current_time->tm_hour >= 12)
      {
         if (night_enabled == false)
         {
            set_container_image(&time_digits_images[5], RESOURCE_ID_IMAGE_PM_MODE, GPoint(93 + time_x_offset, time_y_offset));
         }
         else
         {
            set_container_image(&time_digits_images[5], RESOURCE_ID_IMAGE_INV_PM_MODE, GPoint(93 + time_x_offset, time_y_offset));
         }
      }
      else
      {
         if (night_enabled == false)
         {
            set_container_image(&time_digits_images[5], RESOURCE_ID_IMAGE_AM_MODE, GPoint(93 + time_x_offset, time_y_offset));
         }
         else
         {
            set_container_image(&time_digits_images[5], RESOURCE_ID_IMAGE_INV_AM_MODE, GPoint(93 + time_x_offset, time_y_offset));
         }
      }

      if ((current_time->tm_hour % 12) == 0)
      {
         if (night_enabled == false)
         {
            set_container_image(&time_digits_images[0], BIG_DIGIT_IMAGE_RESOURCE_IDS[1], GPoint(time_x_offset, time_y_offset));
            set_container_image(&time_digits_images[1], BIG_DIGIT_IMAGE_RESOURCE_IDS[2], GPoint(21 + time_x_offset, time_y_offset));
         }
         else
         {
            set_container_image(&time_digits_images[0], BIG_DIGIT_INV_IMAGE_RESOURCE_IDS[1], GPoint(time_x_offset, time_y_offset));
            set_container_image(&time_digits_images[1], BIG_DIGIT_INV_IMAGE_RESOURCE_IDS[2], GPoint(21 + time_x_offset, time_y_offset));
         }
      }
      else
      {
         if (night_enabled == false)
         {
            set_container_image(&time_digits_images[0], BIG_DIGIT_IMAGE_RESOURCE_IDS[(current_time->tm_hour % 12) / 10], GPoint(time_x_offset, time_y_offset));
            set_container_image(&time_digits_images[1], BIG_DIGIT_IMAGE_RESOURCE_IDS[(current_time->tm_hour % 12) % 10], GPoint(21 + time_x_offset, time_y_offset));
         }
         else
         {
            set_container_image(&time_digits_images[0], BIG_DIGIT_INV_IMAGE_RESOURCE_IDS[(current_time->tm_hour % 12) / 10], GPoint(time_x_offset, time_y_offset));
            set_container_image(&time_digits_images[1], BIG_DIGIT_INV_IMAGE_RESOURCE_IDS[(current_time->tm_hour % 12) % 10], GPoint(21 + time_x_offset, time_y_offset));
         }

         if ((current_time->tm_hour % 12) < 10)
         {
            if (night_enabled == false)
            {
               set_container_image(&time_digits_images[0], RESOURCE_ID_IMAGE_NUM_BLANK, GPoint(time_x_offset, time_y_offset));
            }
            else
            {
               set_container_image(&time_digits_images[0], RESOURCE_ID_IMAGE_NUM_INV_BLANK, GPoint(time_x_offset, time_y_offset));
            }
         }
      }
   }

   // display colon & time minute
   if (night_enabled == false)
   {
      set_container_image(&time_digits_images[2], RESOURCE_ID_IMAGE_COLON, GPoint(42 + time_x_offset, time_y_offset));
      set_container_image(&time_digits_images[3], BIG_DIGIT_IMAGE_RESOURCE_IDS[current_time->tm_min / 10], GPoint(51 + time_x_offset, time_y_offset));
      set_container_image(&time_digits_images[4], BIG_DIGIT_IMAGE_RESOURCE_IDS[current_time->tm_min % 10], GPoint(72 + time_x_offset, time_y_offset));
   }
   else
   {
      set_container_image(&time_digits_images[2], RESOURCE_ID_IMAGE_INV_COLON, GPoint(42 + time_x_offset, time_y_offset));
      set_container_image(&time_digits_images[3], BIG_DIGIT_INV_IMAGE_RESOURCE_IDS[current_time->tm_min / 10], GPoint(51 + time_x_offset, time_y_offset));
      set_container_image(&time_digits_images[4], BIG_DIGIT_INV_IMAGE_RESOURCE_IDS[current_time->tm_min % 10], GPoint(72 + time_x_offset, time_y_offset));
   }
}  // update_time()


void pbl_main(void *params)
{
   PebbleAppHandlers handlers =
   {
      .init_handler = &handle_init,
      .deinit_handler = &handle_deinit,

      .tick_info =
      {
         .tick_handler = &handle_second_tick,
         .tick_units = SECOND_UNIT
      }
   };

   app_event_loop(params, &handlers);
}  // pbl_main()







