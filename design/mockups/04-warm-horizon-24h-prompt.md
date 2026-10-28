# Warm Horizon — 24-hour forecast

Generated using the built-in image_gen tool, with Paper as the appearance reference and Horizon as the layout reference. Sample Celsius readings. Firmware unchanged.

Image: 04-warm-horizon-24h.png

## Generation prompt

Use case: ui-mockup
Asset type: revised kitchen weather station UI concept, flat landscape screen for a 5-inch 800 x 480 touch display.
Input images: Image 1 (02-paper.png) is the palette and visual style reference. Image 2 (03-horizon.png) is the layout edit target.
Primary request: Revise Image 2 to use the warm appearance of Image 1 and expand its lower forecast to the next 24 hours. Preserve the basic hierarchy and positions of Horizon: date upper left, enormous clock upper left, current outdoor weather upper right, quiet indoor sensor line below clock, forecast occupying lower portion. Exactly one full-screen design, 5:3 aspect ratio, UI fills canvas edge to edge, no physical device or presentation border.
Appearance: warm ivory #F2EDE3 background, deep charcoal #292927 lettering, slightly darker warm oatmeal #E7E0D3 lower forecast band, soft burnt orange #B96C40 for sunshine and temperature trend, muted slate #64758A for night moons. Use Paper reference's crisp simple weather icons and warm understated typography. No blue background, no white text, no shadows, no texture, no photorealism, no glossy cards.
Top line: "Tuesday, October 27" at left and small charcoal Wi-Fi and gear icons at right. Main area: huge dark "8:42" with modest "AM", approximately same proportions as Horizon. Below it "Inside 21° · Humidity 42%". Main upper right: sun/cloud icon with large "6°", "Outside now", and "Feels like 3°".
Lower forecast band: about 190 logical pixels high, begins near y290. Header left "NEXT 24 HOURS"; right "Tonight −2° / Tomorrow 9°". A single row of EIGHT equally spaced forecast samples. Each column has time at top, weather icon, a thin continuous smooth temperature trend line with dot, then large temperature below. At logical 800px each column is about 90px wide, use compact readable time labels about 18–20px, temperatures 30–32px, icons 34–40px. Display all eight simultaneously; no scrolling, no pagination.
Exact eight times and temperatures from left to right:
"9 AM" "6°" partly cloudy;
"12 PM" "8°" sunny;
"3 PM" "7°" cloudy;
"6 PM" "3°" cloudy;
"9 PM" "1°" crescent with cloud;
"12 AM" "−1°" clear crescent;
"3 AM" "−2°" clear crescent;
"6 AM" "0°" crescent with cloud.
The curve rises slightly from 6 to 8, falls gradually toward the overnight −2 low, then rises to 0. Curve should be subordinate, hairline muted terracotta, not a chart with axes. A subtle vertical separator immediately before 12 AM marks midnight, with a tiny readable "WED" label below that column's time if it fits without crowding. Keep large clock first priority; forecast readable on a small screen. Preserve generous outer margins and no clipping. Sample data only. No extra metrics, no tab bar, no rain percentages, no branding, no title outside interface. This must feel like a warm, elegant kitchen clock with a useful complete day-and-night weather outlook.
