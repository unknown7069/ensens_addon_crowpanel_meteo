# Kitchen dashboard fonts

The five `kitchen_*.c` files are numeric subsets of Montserrat Bold, rendered
with four-bit antialiasing. The 188px and 160px faces keep one-digit and
two-digit clock hours on the same baseline. The 104px, 88px and 64px faces adapt
the outdoor temperature to its length. Other labels use the project's existing
LVGL Montserrat fonts.

Source: https://github.com/JulietaUla/Montserrat/blob/master/fonts/ttf/Montserrat-Bold.ttf

License: [SIL Open Font License](Montserrat-OFL.txt).

To regenerate, download `Montserrat-Bold.ttf` from the source above, then run
`lv_font_conv` for each size/name pair (188/kitchen_clock_188,
160/kitchen_clock_160, 104/kitchen_temperature_104,
88/kitchen_temperature_88, 64/kitchen_temperature_64):

```powershell
npm exec --yes --package=lv_font_conv -- lv_font_conv --size 188 --bpp 4 --no-compress --format lvgl --font Montserrat-Bold.ttf --symbols '0123456789:-' --lv-include lvgl.h --lv-font-name kitchen_clock_188 -o kitchen_clock_188.c
```

For the temperature face use `--symbols '0123456789-°'`.
