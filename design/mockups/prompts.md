# Kitchen weather station UI concepts

Generated with the built-in image_gen tool. Concept previews only; firmware unchanged.

Target: 800 × 480 landscape CrowPanel Advance 5.0. All readings are sample values, in Celsius. Horizon's three-hour forecast requires extending the current forecast data model/parser. Night/day condition icons in Midnight and Paper are illustrative and would require forecast condition data to be retained, or neutral night/day icons used.

## A — Midnight

Image: 01-midnight.png

### Generation prompt

Use case: ui-mockup
Asset type: polished flat UI concept for a kitchen weather station on a 5-inch 800 x 480 landscape touch display.
Create ONE full-screen design, exact landscape 5:3 aspect ratio, straight on, UI filling image edge to edge, no bezel, no physical product, no presentation border, no title outside the UI. Design at logical 800 x 480 resolution with type that remains readable on that small display. This is concept A, a sophisticated dark high-contrast 'Midnight' direction.
Priority: time first, weather outlook second, indoor sensor as a quiet supporting detail. Calm, beautiful, practical appliance interface readable across a kitchen. Flat near-black navy #10151B background, ivory typography, amber sun/weather accents and muted cool blue overnight low. No gradients, no shadows, no glass, no excessive cards.
Layout: 32px outer margins. Quiet top line 'TUESDAY, OCTOBER 27' left; tiny Wi-Fi and gear settings icons at top right. Main upper area about 270px high: left 58% devoted to very large bold elegant sans-serif clock '8:42' about 152px logical size, small 'AM' aligned beside it. Below clock one restrained line 'Inside 21°  ·  Humidity 42%'. Right of a fine vertical separator a crisp simple sun-behind-cloud icon, large '6°', and clear label 'Outside now', then 'Partly cloudy' and 'Feels like 3°'. Clock must dominate.
Bottom area separated by fine hairline at y 325; two generously spaced horizontal forecast groups, NOT tiny dense widgets. Left group a simple moon icon, label 'TONIGHT', large '−2°', supporting label 'Low'. Right group sun/cloud icon, label 'TOMORROW', large '9°', supporting label 'High'. Plenty of spacing, strong hierarchy, aligned baseline, 24px minimum meaningful labels. Small tappable gear at least 40px logical area. Use sample data exactly. No permanent tab bar, no navigation sidebar, no extra metrics, no logos, no seconds, no ornamental graphics. Clean precise embedded LVGL-feasible interface. Make it feel intentionally designed, not a generic admin dashboard.

## B — Paper

Image: 02-paper.png

### Generation prompt

Use case: ui-mockup
Asset type: polished flat UI concept for a kitchen weather station on a 5-inch 800 x 480 landscape touch display.
Create ONE full-screen design, exact landscape 5:3 aspect ratio, straight on, UI filling image edge to edge, no bezel or physical device, no presentation border or captions outside UI. Respect logical 800 x 480 resolution, large type readable at a glance. This is concept B, warm 'Paper' direction.
Design language: warm ivory #F2EDE3 background, deep charcoal typography, restrained burnt orange #B96C40 accent, muted slate blue moon icon. Refined contemporary kitchen appliance, typographic and calm, subtle rules, completely flat. No textures, shadows, gradients or glossy cards.
Clock first, upcoming weather second. Quiet date 'Tuesday, October 27' centered at top with a small discrete gear in upper right. Enormous centered geometric sans-serif clock '8:42' occupying the upper middle 50% of screen height, approx 154px logical font height, modest 'AM' at right of clock aligned low. Clock should be the first thing visible across the kitchen. Under clock a readable quiet line 'Inside 21°  ·  Humidity 42%'.
Bottom third about 150px tall is a lightly tinted #E7E0D3 continuous forecast band divided into three generous equal columns using thin vertical lines. First column a simple cloud/sun icon with 'NOW', large '6°', small 'Partly cloudy'. Second a simple crescent icon with 'TONIGHT', large '−2°', small 'Low'. Third a simple sun/cloud icon with 'TOMORROW', large '9°', small 'High'. Thoughtful alignment and generous whitespace, icons all same simple vector style, labels at least 20px logical height, temperatures 40px, dark text high contrast. No tab bar, no navigation rails, no seconds, no names or brands, no extra metrics or explanatory copy. Render a complete elegant UI ready to discuss, with all text clear and exact.

## C — Horizon

Image: 03-horizon.png

### Generation prompt

Use case: ui-mockup
Asset type: polished flat UI concept for a kitchen weather station on a 5-inch 800 x 480 landscape touch display.
Create ONE full-screen design, exact landscape 5:3 aspect ratio, straight on, edge-to-edge interface only, no bezel, physical object, presentation border, external titles. Layout based on logical 800 x 480, no microtype. This is concept C, 'Horizon', a weather-ahead design.
Style: deep desaturated blue #18323F background, clean warm-white typography, restrained soft turquoise accent #9DD3CF, amber sun icon accents. Distinct beautiful modern embedded appliance UI, minimal flat vector rendering, no glossy cards, no photography, no scenery, no 3D icons. Prioritize a huge readable clock and a simple near-term forecast.
Top 30px quiet date 'Tuesday, October 27' at left, small Wi-Fi and gear at right. Main upper 240px: huge '8:42' in strong rounded modern sans, about 144px logical size, small 'AM' next to it, clock taking left 60%. Below clock a clear quiet line 'Inside 21°  ·  42% RH'. Upper right weather group: simple partly cloudy line-and-fill icon, large '6°', then 'Outside now' and 'Feels like 3°'. Clock absolutely dominant.
Lower 185px separated by thin line. Header at left 'NEXT 12 HOURS'. Header at right 'Tonight −2°  /  Tomorrow 9°'. Under it four evenly spaced forecast columns, not cards: first '9 AM' with partly cloudy icon and '6°'; second '12 PM' with sunny icon and '8°'; third '3 PM' with cloudy icon and '7°'; fourth '6 PM' with cloudy icon and '3°'. Simple subtle connecting temperature trend line behind or above the four column temperatures, not a full chart, no axes. All meaningful labels at least 20px logical size, forecast temperatures 34px, icons large and crisp. Clear structure and breathing room, attractive and implementable in LVGL. No fake rain percentages, no tab bar, no sidebar, no seconds, no logos, no extra copy. Sample data as specified.
